// kernel.c — ядро Manpupuner_42 (v0.2-engine) — БЕЗ IDT
#include <stdint.h>
#include <stddef.h>

// ============================================================
// 1. VGA-вывод
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
    char* args = (char*)&format + sizeof(format);
    while (*format) {
        if (*format == '%' && *(format + 1)) {
            format++;
            if (*format == 'c') {
                char c = *((char*)args);
                args += sizeof(char);
                putchar(c);
            } else if (*format == 's') {
                char* s = *((char**)args);
                args += sizeof(char*);
                while (*s) {
                    putchar(*s);
                    s++;
                }
            } else if (*format == 'd') {
                int num = *((int*)args);
                args += sizeof(int);
                char buf[16];
                itoa(num, buf);
                char* p = buf;
                while (*p) {
                    putchar(*p);
                    p++;
                }
            } else if (*format == 'x') {
                uint32_t num = *((uint32_t*)args);
                args += sizeof(uint32_t);
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
// 3. Планировщик процессов
// ============================================================

#define MAX_PROCESSES 16
#define PROCESS_STACK_SIZE 4096

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

    uint32_t old_pid = current_pid;
    do {
        current_pid = (current_pid + 1) % MAX_PROCESSES;
    } while (processes[current_pid].state != PROCESS_STATE_READY);

    process_t* old = &processes[old_pid];
    process_t* new = &processes[current_pid];

    old->state = PROCESS_STATE_READY;
    new->state = PROCESS_STATE_RUNNING;

    switch_context(&old->esp, &old->eip, new->esp, new->eip);
}

// ============================================================
// 4. Виртуальная ФС
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

    extern uint8_t _binary_modules_keyboard_module_bin_start[];
    extern uint8_t _binary_modules_keyboard_module_bin_end[];
    int size = _binary_modules_keyboard_module_bin_end - _binary_modules_keyboard_module_bin_start;
    fs_add_file("keyboard.bin", (const char*)_binary_modules_keyboard_module_bin_start, size);
}

// ============================================================
// 5. /dev/keyboard
// ============================================================

#define DEV_KEYBOARD 100
static char keyboard_buffer[256];
static int kb_head = 0;
static int kb_tail = 0;

int sys_read_keyboard(char* buffer, uint32_t size) {
    int count = 0;
    while (kb_head != kb_tail && count < size) {
        buffer[count++] = keyboard_buffer[kb_tail];
        kb_tail = (kb_tail + 1) % 256;
    }
    return count;
}

int sys_write_file_wrapper(uint32_t id, const char* data, uint32_t size) {
    if (id == DEV_KEYBOARD) {
        for (uint32_t i = 0; i < size; i++) {
            keyboard_buffer[kb_head] = data[i];
            kb_head = (kb_head + 1) % 256;
        }
        return size;
    }
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

// ============================================================
// 6. Менеджер памяти
// ============================================================

#define MAX_MEMORY_BLOCKS 32

typedef struct {
    uint32_t id;
    uint32_t size;
    int used;
} memory_block_t;

static memory_block_t memory_blocks[MAX_MEMORY_BLOCKS];

void memory_init() {
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        memory_blocks[i].used = 0;
        memory_blocks[i].id = 0;
        memory_blocks[i].size = 0;
    }
}

// ============================================================
// 7. Системные вызовы (7 штук)
// ============================================================

int sys_read_file(uint32_t id, char* buffer, uint32_t size) {
    if (id == DEV_KEYBOARD) {
        return sys_read_keyboard(buffer, size);
    }
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
    return sys_write_file_wrapper(id, data, size);
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

uint32_t sys_alloc_memory(uint32_t size) {
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        if (!memory_blocks[i].used) {
            memory_blocks[i].id = i + 1;
            memory_blocks[i].size = size;
            memory_blocks[i].used = 1;
            printk("Allocated block %d (%d bytes)\n", i + 1, size);
            return i + 1;
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
            printk("Freed block %d\n", id);
            return 0;
        }
    }
    printk("Memory block %d not found\n", id);
    return -1;
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

    process_t* p = &processes[slot];
    p->pid = slot + 1;
    p->state = PROCESS_STATE_READY;
    p->esp = (uint32_t)(stack + PROCESS_STACK_SIZE - 1);
    p->eip = (uint32_t)entry;

    int i = 0;
    while (name[i] && i < 31) {
        p->name[i] = name[i];
        i++;
    }
    p->name[i] = '\0';

    process_count++;
    printk("Process %d '%s' created, entry at 0x%x\n", p->pid, p->name, p->eip);
    return p->pid;
}

// ============================================================
// 8. Тестовый процесс
// ============================================================

void test_process() {
    while (1) {
        printk(".");
        for (volatile int i = 0; i < 100000; i++);
    }
}

// ============================================================
// 9. Загрузчик модулей
// ============================================================

#define MODULE_MAGIC 0x4D504B32

struct module_header {
    uint32_t magic;
    uint32_t entry_point;
    uint32_t data_size;
};

void load_module(const char* filename) {
    printk("Loading module: %s\n", filename);

    // 1. Ищем файл
    uint32_t file_id = 0;
    uint32_t count;
    uint32_t* ids = sys_list_files(&count);
    int found = 0;

    for (uint32_t i = 0; i < count; i++) {
        file_t* f = fs_get_file(ids[i]);
        int match = 1;
        for (int j = 0; j < 32; j++) {
            if (f->name[j] != filename[j]) { match = 0; break; }
            if (f->name[j] == '\0') break;
        }
        if (match) {
            file_id = ids[i];
            found = 1;
            break;
        }
    }

    if (!found) {
        printk("Module file not found: %s\n", filename);
        return;
    }

    // 2. Читаем файл
    char buffer[512];
    int size = sys_read_file(file_id, buffer, 512);
    if (size < sizeof(struct module_header)) {
        printk("Module too small or corrupted\n");
        return;
    }

    // 3. Проверяем магическое число
    struct module_header* header = (struct module_header*)buffer;
    if (header->magic != MODULE_MAGIC) {
        printk("Invalid module magic: 0x%x (expected 0x%x)\n", header->magic, MODULE_MAGIC);
        return;
    }

    printk("Module loaded: magic=0x%x, entry=0x%x, data_size=%d\n",
           header->magic, header->entry_point, header->data_size);

    // 4. Подготавливаем стек для модуля
    uint32_t* module_stack = (uint32_t*)sys_alloc_memory(4096);
    if (!module_stack) {
        printk("Failed to allocate stack for module\n");
        return;
    }

    // 5. Вычисляем точку входа
    void (*entry)() = (void (*)())((uint32_t)header + header->entry_point);
    printk("Starting module at 0x%x\n", entry);

    // 6. Передаём системные вызовы в модуль (глобальные переменные)
    // Для этого нужно, чтобы модуль знал адреса sys_write_file и sys_read_file
    // Пока просто вызываем entry
    entry();
}

// ============================================================
// 10. Emergency shell
// ============================================================

void reboot() {
    printk("Rebooting...\n");
    asm volatile("int $0x19");
}

void dump() {
    printk("Dump: memory not implemented yet.\n");
}

void print_help() {
    printk("Commands:\n");
    printk("  reboot      - restart system\n");
    printk("  dump        - memory dump (stub)\n");
    printk("  load <file> - load module\n");
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
    printk("Manpupuner_42 v0.2-engine\n");
    printk("Built: 25.06.2026\n");
}

void print_syscalls() {
    printk("7 system calls:\n");
    printk("  1. READ_FILE\n");
    printk("  2. WRITE_FILE\n");
    printk("  3. CREATE_PROCESS\n");
    printk("  4. SLEEP\n");
    printk("  5. ALLOC_MEMORY\n");
    printk("  6. FREE_MEMORY\n");
    printk("  7. LIST_FILES\n");
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
    if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'b' && cmd[3] == 'o' && cmd[4] == 'o' && cmd[5] == 't') {
        reboot();
    } else if (cmd[0] == 'd' && cmd[1] == 'u' && cmd[2] == 'm' && cmd[3] == 'p') {
        dump();
    } else if (cmd[0] == 'l' && cmd[1] == 'o' && cmd[2] == 'a' && cmd[3] == 'd') {
        char* name = cmd + 5;
        while (*name == ' ') name++;
        if (*name) {
            load_module(name);
        } else {
            printk("Usage: load <filename>\n");
        }
    } else if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p') {
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
            sys_create_process(name, 0);
        } else {
            printk("Usage: run <name>\n");
        }
    } else {
        printk("Unknown command. Type 'help' for list.\n");
    }
}

// ============================================================
// 11. Клавиатура (временное решение)
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
        if ((inb(KEYBOARD_STATUS) & 1) == 0) continue;
        uint8_t scancode = inb(KEYBOARD_PORT);

        if (scancode == 0xAA || scancode == 0xB6) { shift = 0; continue; }
        if (scancode == 0x2A || scancode == 0x36) { shift = 1; continue; }

        if (scancode & 0x80) continue;

        char c = 0;
        if (scancode == 0x39) c = ' ';
        else if (scancode == 0x1C) c = '\n';
        else if (scancode == 0x0E) c = '\b';
        else if (scancode < sizeof(scancode_table)) {
            c = scancode_table[scancode];
        }

        if (shift) {
            if (c == '-') c = '_';
            else if (c == '/') c = '?';
            else if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        }

        if (c) return c;
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
// 12. Точка входа
// ============================================================

void kernel_main() {
    clear_screen();
    uart_init();
    init_filesystem();
    memory_init();
    process_init();

    printk("========================================\n");
    printk("Manpupuner_42 v0.2-engine (UART enabled)\n");
    printk("========================================\n");
    uart_printk("[UART] Kernel started\n");

    sys_create_process("test", test_process);

    char cmd[256];
    while (1) {
        printk("> ");
        readline(cmd, 256);
        handle_command(cmd);
    }
}
