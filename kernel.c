// kernel.c — Manpupuner_42 v0.3-alpha
// Minimal hybrid arbiter kernel with 7 syscalls, IDT, timer, and multitasking
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

// ============================================================
// Function prototypes
// ============================================================

void test_process();
void shell_process();
void readline(char* buffer, int max_len);
void handle_command(char* cmd);
void yield();
void print_help();
void print_version();
void print_syscalls();
void cmd_ls();
void cmd_read(uint32_t id);
void cmd_write(uint32_t id, const char* data);

// ============================================================
// 1. VGA output
// ============================================================

#define VGA_MEMORY ((char*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static int cursor_x = 0;
static int cursor_y = 0;

void scroll_screen() {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int src = (y * VGA_WIDTH + x) * 2;
            int dst = ((y - 1) * VGA_WIDTH + x) * 2;
            VGA_MEMORY[dst] = VGA_MEMORY[src];
            VGA_MEMORY[dst + 1] = VGA_MEMORY[src + 1];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        int pos = ((VGA_HEIGHT - 1) * VGA_WIDTH + x) * 2;
        VGA_MEMORY[pos] = ' ';
        VGA_MEMORY[pos + 1] = 0x07;
    }
    cursor_y = VGA_HEIGHT - 1;
}

void putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) scroll_screen();
        return;
    }
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
            VGA_MEMORY[pos] = ' ';
            VGA_MEMORY[pos + 1] = 0x07;
        }
        return;
    }
    int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
    VGA_MEMORY[pos] = c;
    VGA_MEMORY[pos + 1] = 0x07;
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) scroll_screen();
    }
}

void itoa(int num, char* buffer) {
    char temp[16];
    int i = 0;
    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    while (num > 0) {
        temp[i++] = '0' + (num % 10);
        num /= 10;
    }
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

void printk(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    while (*format) {
        if (*format == '%' && *(format + 1)) {
            format++;
            if (*format == 'c') {
                char c = (char)va_arg(args, int);
                putchar(c);
            } else if (*format == 's') {
                char* s = va_arg(args, char*);
                while (*s) {
                    putchar(*s);
                    s++;
                }
            } else if (*format == 'd') {
                int num = va_arg(args, int);
                char buf[16];
                itoa(num, buf);
                char* p = buf;
                while (*p) {
                    putchar(*p);
                    p++;
                }
            } else if (*format == 'x') {
                uint32_t num = va_arg(args, uint32_t);
                const char hex[] = "0123456789ABCDEF";
                char buf[16];
                buf[0] = '0';
                buf[1] = 'x';
                int pos = 2;
                for (int i = 28; i >= 0; i -= 4) {
                    buf[pos++] = hex[(num >> i) & 0xF];
                }
                buf[pos] = '\0';
                char* p = buf;
                while (*p) {
                    putchar(*p);
                    p++;
                }
            } else {
                putchar('%');
                putchar(*format);
            }
        } else {
            putchar(*format);
        }
        format++;
    }
    va_end(args);
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i++) {
        VGA_MEMORY[i] = 0;
    }
    cursor_x = 0;
    cursor_y = 0;
}

// ============================================================
// 2. UART
// ============================================================

#define UART_PORT 0x3F8

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void uart_init() {
    outb(UART_PORT + 1, 0x00);
    outb(UART_PORT + 3, 0x80);
    outb(UART_PORT + 0, 0x03);
    outb(UART_PORT + 1, 0x00);
    outb(UART_PORT + 3, 0x03);
    outb(UART_PORT + 2, 0xC7);
    outb(UART_PORT + 4, 0x0B);
}

void uart_putchar(char c) {
    while (!(inb(UART_PORT + 5) & 0x20));
    outb(UART_PORT, c);
}

void uart_printk(const char* str) {
    while (*str) {
        if (*str == '\n') uart_putchar('\r');
        uart_putchar(*str);
        str++;
    }
}

// ============================================================
// 3. IDT
// ============================================================

#define IDT_SIZE 256
#define IRQ0 32

typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_descriptor_t;

idt_entry_t idt[IDT_SIZE];
idt_descriptor_t idt_desc;

extern void load_idt();
extern void reload_idt();
extern void irq_handler();

void set_idt_gate(int num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].reserved = 0;
    idt[num].flags = flags;
}

void init_idt() {
    for (int i = 0; i < IDT_SIZE; i++) {
        set_idt_gate(i, 0, 0, 0);
    }
    uint32_t handler_addr = (uint32_t)irq_handler;
    printk("irq_handler address: 0x%x\n", handler_addr);
    set_idt_gate(IRQ0, handler_addr, 0x10, 0x8E);
    idt_desc.limit = IDT_SIZE * sizeof(idt_entry_t) - 1;
    idt_desc.base = (uint32_t)&idt;
    load_idt();
    printk("IDT loaded\n");
}

// ============================================================
// 4. PIC
// ============================================================

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

void pic_remap() {
    outb(PIC_MASTER_CMD, 0x11);
    outb(PIC_SLAVE_CMD, 0x11);
    outb(PIC_MASTER_DATA, 0x20);
    outb(PIC_SLAVE_DATA, 0x28);
    outb(PIC_MASTER_DATA, 0x04);
    outb(PIC_SLAVE_DATA, 0x02);
    outb(PIC_MASTER_DATA, 0x01);
    outb(PIC_SLAVE_DATA, 0x01);
    outb(PIC_MASTER_DATA, 0xFE);
    outb(PIC_SLAVE_DATA, 0xFF);
    printk("PIC remapped: IRQ0 → vector 0x20\n");
}

// ============================================================
// 5. Timer (PIT)
// ============================================================

#define PIT_PORT 0x40
#define PIT_CMD_PORT 0x43

static volatile int need_yield = 0;
static volatile int irq_count = 0;

void irq_handler_c() {
    irq_count++;
    if (irq_count % 10 == 0) {
        printk("!");
    }
    reload_idt();
    need_yield = 1;
    outb(0x20, 0x20);
}

void init_timer() {
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_PORT, 0x00);
    outb(PIT_PORT, 0x00);
    printk("Timer initialized (100 Hz)\n");
}

// ============================================================
// 6. Scheduler
// ============================================================

#define MAX_PROCESSES 16
#define PROCESS_STACK_SIZE 8192

typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED
} process_state_t;

typedef struct {
    uint32_t pid;
    char name[32];
    process_state_t state;
    uint32_t esp;
    uint32_t eip;
} process_t;

static process_t processes[MAX_PROCESSES];
static uint32_t process_count = 0;
static uint32_t current_pid = 0;

extern void switch_context(uint32_t* old_esp, uint32_t* old_eip, 
                           uint32_t new_esp, uint32_t new_eip);

void process_init() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = 0;
        processes[i].state = PROCESS_STATE_BLOCKED;
        processes[i].esp = 0;
        processes[i].eip = 0;
        processes[i].name[0] = '\0';
    }
    process_count = 0;
    current_pid = 0;
}

void yield() {
    if (process_count < 2) return;

    uint32_t next_pid = (current_pid + 1) % MAX_PROCESSES;
    uint32_t start_pid = next_pid;

    do {
        if (processes[next_pid].state == PROCESS_STATE_READY) {
            break;
        }
        next_pid = (next_pid + 1) % MAX_PROCESSES;
    } while (next_pid != start_pid);

    if (processes[next_pid].state != PROCESS_STATE_READY) {
        return;
    }

    process_t* old = &processes[current_pid];
    process_t* new = &processes[next_pid];

    if (old == new) return;

    old->state = PROCESS_STATE_READY;
    new->state = PROCESS_STATE_RUNNING;
    current_pid = next_pid;

    switch_context(&old->esp, &old->eip, new->esp, new->eip);
}

// ============================================================
// 7. Virtual filesystem
// ============================================================

#define MAX_FILES 16
#define MAX_FILENAME 32
#define MAX_CONTENT 256

typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_CONTENT];
    uint32_t size;
} file_t;

static file_t files[MAX_FILES];
static uint32_t file_count = 0;

void fs_add_file(const char* name, const char* content, uint32_t size) {
    if (file_count >= MAX_FILES) return;
    file_t* f = &files[file_count++];
    int i = 0;
    while (name[i] && i < MAX_FILENAME - 1) {
        f->name[i] = name[i];
        i++;
    }
    f->name[i] = '\0';
    for (i = 0; i < size && i < MAX_CONTENT - 1; i++) {
        f->content[i] = content[i];
    }
    f->content[i] = '\0';
    f->size = size;
}

file_t* fs_get_file(uint32_t id) {
    if (id >= file_count) return 0;
    return &files[id];
}

uint32_t* fs_list_files(uint32_t* count) {
    static uint32_t ids[MAX_FILES];
    for (int i = 0; i < file_count; i++) {
        ids[i] = i;
    }
    *count = file_count;
    return ids;
}

void init_filesystem() {
    fs_add_file("/home/hello.txt", "Hello from Manpupuner_42!", 27);
    fs_add_file("/README.md", "# Manpupuner_42\n\nPure Kernel Demo\n\n7 syscalls", 50);
    fs_add_file("/etc/hosts", "127.0.0.1 localhost", 20);
}

// ============================================================
// 8. Memory manager
// ============================================================

#define MAX_MEMORY_BLOCKS 32
#define KERNEL_HEAP_START 0x00108000

typedef struct {
    uint32_t id;
    uint32_t size;
    int used;
    void* addr;
} memory_block_t;

static memory_block_t memory_blocks[MAX_MEMORY_BLOCKS];
static uint32_t memory_counter = 0;
static uint32_t heap_top = KERNEL_HEAP_START;

void memory_init() {
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        memory_blocks[i].used = 0;
        memory_blocks[i].id = 0;
        memory_blocks[i].size = 0;
        memory_blocks[i].addr = 0;
    }
    memory_counter = 0;
    heap_top = KERNEL_HEAP_START;
}

void* sys_alloc_memory(uint32_t size) {
    size = (size + 3) & ~3;
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        if (!memory_blocks[i].used) {
            void* addr = (void*)heap_top;
            heap_top += size;
            memory_counter++;
            memory_blocks[i].id = memory_counter;
            memory_blocks[i].size = size;
            memory_blocks[i].used = 1;
            memory_blocks[i].addr = addr;
            printk("Allocated block %d at 0x%x (%d bytes)\n", 
                   memory_blocks[i].id, addr, size);
            return addr;
        }
    }
    printk("Memory allocation failed: no free blocks\n");
    return 0;
}

int sys_free_memory(uint32_t id) {
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        if (memory_blocks[i].id == id && memory_blocks[i].used) {
            memory_blocks[i].used = 0;
            memory_blocks[i].id = 0;
            memory_blocks[i].size = 0;
            memory_blocks[i].addr = 0;
            printk("Freed block %d\n", id);
            return 0;
        }
    }
    printk("Memory block %d not found\n", id);
    return -1;
}

// ============================================================
// 9. System calls (7 total)
// ============================================================

int sys_read_file(uint32_t id, char* buffer, uint32_t size) {
    file_t* f = fs_get_file(id);
    if (!f) return -1;
    uint32_t read_size = size;
    if (read_size > f->size) read_size = f->size;
    for (uint32_t i = 0; i < read_size; i++) {
        buffer[i] = f->content[i];
    }
    return read_size;
}

int sys_write_file(uint32_t id, const char* data, uint32_t size) {
    file_t* f = fs_get_file(id);
    if (!f) return -1;
    uint32_t write_size = size;
    if (write_size > MAX_CONTENT - 1) write_size = MAX_CONTENT - 1;
    for (uint32_t i = 0; i < write_size; i++) {
        f->content[i] = data[i];
    }
    f->content[write_size] = '\0';
    f->size = write_size;
    return write_size;
}

uint32_t* sys_list_files(uint32_t* count) {
    return fs_list_files(count);
}

void sys_sleep(uint32_t milliseconds) {
    for (uint32_t i = 0; i < milliseconds * 1000; i++) {
        for (volatile int j = 0; j < 1000; j++) {
        }
    }
}

uint32_t sys_create_process(const char* name, void (*entry)()) {
    if (process_count >= MAX_PROCESSES) {
        printk("Maximum processes reached\n");
        return 0;
    }
    
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_STATE_BLOCKED && processes[i].pid == 0) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printk("No free process slot\n");
        return 0;
    }

    uint32_t* stack = (uint32_t*)sys_alloc_memory(PROCESS_STACK_SIZE);
    if (!stack) {
        printk("Failed to allocate stack for process\n");
        return 0;
    }

    for (int i = 0; i < PROCESS_STACK_SIZE / 4; i++) {
        stack[i] = 0;
    }
    
    uint32_t* local_esp = (uint32_t*)((uint32_t)stack + PROCESS_STACK_SIZE);

    local_esp--; 
    *local_esp = (uint32_t)entry; 

    local_esp--; *local_esp = 0;
    local_esp--; *local_esp = 0;
    local_esp--; *local_esp = 0;
    local_esp--; *local_esp = 0;

    process_t* p = &processes[slot];
    p->pid = slot + 1;
    p->state = PROCESS_STATE_READY;
    p->esp = (uint32_t)local_esp;
    p->eip = (uint32_t)entry;
    
    int i = 0;
    while (name[i] && i < 31) {
        p->name[i] = name[i];
        i++;
    }
    p->name[i] = '\0';
    
    process_count++;
    printk("Process %d '%s' created, entry at 0x%x, stack at 0x%x\n", p->pid, p->name, p->eip, p->esp);
    return p->pid;
}

// ============================================================
// 10. Keyboard (asynchronous with yield)
// ============================================================

#define KEYBOARD_PORT 0x60
#define KEYBOARD_STATUS 0x64

static const char scancode_table[] = {
    0,   0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
};

char read_key() {
    static int shift = 0;
    while (1) {
        if ((inb(KEYBOARD_STATUS) & 1) == 0) {
            yield();
            continue;
        }
        
        uint8_t scancode = inb(KEYBOARD_PORT);

        if (scancode == 0x2A || scancode == 0x36) { shift = 1; continue; }
        if (scancode == 0xAA || scancode == 0xB6) { shift = 0; continue; }

        if (scancode == 0xE0) {
            while ((inb(KEYBOARD_STATUS) & 1) == 0) { yield(); }
            uint8_t ext = inb(KEYBOARD_PORT);
            if (ext == 0x48) return 'A';
            if (ext == 0x50) return 'B';
            if (ext == 0x4B) return 'D';
            if (ext == 0x4D) return 'C';
            continue;
        }

        if (scancode & 0x80) continue;

        char c = 0;
        if (scancode == 0x39) c = ' ';
        else if (scancode == 0x1C) c = '\n';
        else if (scancode == 0x0E) c = '\b';
        else if (scancode < sizeof(scancode_table)) {
            c = scancode_table[scancode];
        }

        if (c == 0) continue;

        if (c >= 'a' && c <= 'z') {
            if (shift) c = c - 'a' + 'A';
        } else if (shift) {
            switch (c) {
                case '1': c = '!'; break; case '2': c = '@'; break; case '3': c = '#'; break;
                case '4': c = '$'; break; case '5': c = '%'; break; case '6': c = '^'; break;
                case '7': c = '&'; break; case '8': c = '*'; break; case '9': c = '('; break;
                case '0': c = ')'; break; case '-': c = '_'; break; case '=': c = '+'; break;
                case '[': c = '{'; break; case ']': c = '}'; break; case '\\': c = '|'; break;
                case ';': c = ':'; break; case '\'': c = '"'; break; case ',': c = '<'; break;
                case '.': c = '>'; break; case '/': c = '?'; break;
            }
        }

        return c;
    }
}

void readline(char* buffer, int max_len) {
    int i = 0;
    while (1) {
        char c = read_key();
        if (c == '\n') {
            buffer[i] = '\0';
            printk("\n");
            return;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                printk("\b \b");
            }
        } else if (i < max_len - 1) {
            buffer[i++] = c;
            printk("%c", c);
        }
    }
}

// ============================================================
// 11. Processes
// ============================================================

void test_process() {
    while (1) {
        printk(".");
        for (volatile int i = 0; i < 4000000; i++);
        yield();
    }
}

void shell_process() {
    char cmd[256];
    while (1) {
        printk("S> ");
        readline(cmd, 256);
        handle_command(cmd);
        yield();
    }
}

// ============================================================
// 12. Shell
// ============================================================

void print_help() {
    printk("Commands:\n");
    printk("  help        - this list\n");
    printk("  version     - show kernel version\n");
    printk("  syscalls    - list 7 system calls\n");
    printk("  ls          - list files\n");
    printk("  read <id>   - read file by ID\n");
    printk("  write <id> <text> - write to file\n");
    printk("  alloc <size> - allocate memory\n");
    printk("  free <id>   - free memory\n");
    printk("  run <name>  - create process\n");
}

void print_version() {
    printk("Manpupuner_42 v0.3-alpha\n");
    printk("Built: 30.06.2026\n");
}

void print_syscalls() {
    printk("7 system calls:\n");
    printk("  1. READ_FILE\n  2. WRITE_FILE\n  3. CREATE_PROCESS\n");
    printk("  4. SLEEP\n  5. ALLOC_MEMORY\n  6. FREE_MEMORY\n  7. LIST_FILES\n");
}

void cmd_ls() {
    uint32_t count;
    uint32_t* ids = sys_list_files(&count);
    printk("Files:\n");
    for (uint32_t i = 0; i < count; i++) {
        file_t* f = fs_get_file(ids[i]);
        printk("[%d] %s (%d bytes)\n", ids[i], f->name, f->size);
    }
}

void cmd_read(uint32_t id) {
    char buffer[256];
    int size = sys_read_file(id, buffer, 255);
    if (size > 0) {
        buffer[size] = '\0';
        printk("%s\n", buffer);
    } else {
        printk("File not found\n");
    }
}

void cmd_write(uint32_t id, const char* data) {
    int len = 0;
    while (data[len]) len++;
    int result = sys_write_file(id, data, len);
    if (result >= 0) {
        printk("Written %d bytes\n", result);
    } else {
        printk("File not found\n");
    }
}

void handle_command(char* cmd) {
    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p') {
        print_help();
    } else if (cmd[0] == 'v' && cmd[1] == 'e' && cmd[2] == 'r' && cmd[3] == 's' && cmd[4] == 'i' && cmd[5] == 'o' && cmd[6] == 'n') {
        print_version();
    } else if (cmd[0] == 's' && cmd[1] == 'y' && cmd[2] == 's' && cmd[3] == 'c' && cmd[4] == 'a' && cmd[5] == 'l' && cmd[6] == 'l' && cmd[7] == 's') {
        print_syscalls();
    } else if (cmd[0] == 'l' && cmd[1] == 's') {
        cmd_ls();
    } else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'a' && cmd[3] == 'd') {
        uint32_t id = 0;
        char* p = cmd + 5;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            id = id * 10 + (*p - '0');
            p++;
        }
        cmd_read(id);
    } else if (cmd[0] == 'w' && cmd[1] == 'r' && cmd[2] == 'i' && cmd[3] == 't' && cmd[4] == 'e') {
        uint32_t id = 0;
        char* p = cmd + 6;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            id = id * 10 + (*p - '0');
            p++;
        }
        while (*p == ' ') p++;
        cmd_write(id, p);
    } else if (cmd[0] == 'a' && cmd[1] == 'l' && cmd[2] == 'l' && cmd[3] == 'o' && cmd[4] == 'c') {
        uint32_t size = 0;
        char* p = cmd + 6;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            size = size * 10 + (*p - '0');
            p++;
        }
        sys_alloc_memory(size);
    } else if (cmd[0] == 'f' && cmd[1] == 'r' && cmd[2] == 'e' && cmd[3] == 'e') {
        uint32_t id = 0;
        char* p = cmd + 5;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            id = id * 10 + (*p - '0');
            p++;
        }
        sys_free_memory(id);
    } else if (cmd[0] == 'r' && cmd[1] == 'u' && cmd[2] == 'n') {
        char* name = cmd + 4;
        while (*name == ' ') name++;
        if (*name) {
            sys_create_process(name, test_process);
        } else {
            printk("Usage: run <name>\n");
        }
    } else {
        printk("Unknown command. Type 'help' for list.\n");
    }
}

// ============================================================
// 13. Entry point
// ============================================================

void kernel_main() {
    clear_screen();
    uart_init();
    init_filesystem();
    memory_init();
    process_init();

    printk("Kernel started\n");

    init_idt();
    pic_remap();
    init_timer();

    processes[0].pid = 0;
    processes[0].state = PROCESS_STATE_RUNNING;
    processes[0].name[0] = 'k';
    processes[0].name[1] = 'e';
    processes[0].name[2] = 'r';
    processes[0].name[3] = '\0';
    process_count = 1;
    current_pid = 0;

    printk("Enabling interrupts...\n");
    __asm__ volatile("sti");
    printk("Interrupts enabled\n");

    sys_create_process("shell", shell_process);
    sys_create_process("test1", test_process);
    sys_create_process("test2", test_process);

    while (1) {
        if (need_yield) {
            need_yield = 0;
            yield();
        }
        __asm__ volatile("hlt");
    }
}
