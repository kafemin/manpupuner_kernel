// kernel.c — Manpupuner_42 (Arbiter Kernel)
// ============================================================
// AUTHOR
// ============================================================
//   Idea and architecture author: Kaskov Aleksandr (Kafemin)
//   Technical consultant and code implementer: DeepSeek AI
//   Email: kafemin@gmail.com
//   Project: Manpupuner_42
//   License: MIT
//
//   Description:
//   Manpupuner_42 is a hybrid arbiter kernel that provides
//   only 7 basic system calls. All other logic (file systems,
//   drivers, network, graphics) must be loaded modularly as
//   separate components.
//
//   The project name comes from the Manpupuner plateau in the
//   Northern Urals. Seven stone pillars symbolize the 7 basic
//   system calls. The number 42 refers to the search for a
//   universal answer.
// ============================================================

#include <stdint.h>
#include <stddef.h>

// ============================================================
// 1. VGA (VIDEO MEMORY)
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

void itoa(uint32_t num, char* buffer) {
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

void printk(const char* str, ...) {
    char* args = (char*)&str + sizeof(str);
    while (*str) {
        if (*str == '%' && *(str + 1)) {
            str++;
            if (*str == 'd') {
                uint32_t num = *((uint32_t*)args);
                args += sizeof(uint32_t);
                char buf[16];
                itoa(num, buf);
                for (int i = 0; buf[i]; i++) putchar(buf[i]);
            } else if (*str == 's') {
                char* s = *((char**)args);
                args += sizeof(char*);
                for (int i = 0; s[i]; i++) putchar(s[i]);
            } else if (*str == 'c') {
                char c = *((char*)args);
                args += sizeof(char);
                putchar(c);
            } else {
                putchar('%');
                putchar(*str);
            }
        } else {
            putchar(*str);
        }
        str++;
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
// 2. EXTERNAL KEYBOARD INTERFACE
// ============================================================

extern int keyboard_available();
extern char keyboard_read_key();

// ============================================================
// 3. HELPER FUNCTIONS
// ============================================================

uint32_t atoi(const char* str) {
    uint32_t result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strtok(char* str, const char* delim) {
    static char* last = 0;
    if (str) last = str;
    if (!last || !*last) return 0;
    char* start = last;
    while (*last && (*last != *delim)) last++;
    if (*last) {
        *last = '\0';
        last++;
    }
    return start;
}

// ============================================================
// 4. VIRTUAL FILE SYSTEM
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
    fs_add_file("/README.md", "# Manpupuner_42\n\nHybrid Kernel\n\n7 syscalls", 50);
    fs_add_file("/etc/hosts", "127.0.0.1 localhost", 20);
}

// ============================================================
// 5. 7 BASIC SYSTEM CALLS
// ============================================================
//
// These are the 7 basic calls provided by the kernel.
// All other calls (networking, IPC, memory management, graphics,
// device drivers, file systems) must be loaded modularly as
// separate components running on top of these 7 calls.
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

static uint32_t pid_counter = 0;

uint32_t sys_create_process(const char* name) {
    pid_counter++;
    return pid_counter;
}

void sys_sleep(uint32_t milliseconds) {
    for (uint32_t i = 0; i < milliseconds * 1000; i++) {
        for (volatile int j = 0; j < 1000; j++) {
            // busy-wait
        }
    }
}

static uint32_t memory_counter = 0;

uint32_t sys_alloc_memory(uint32_t size) {
    memory_counter++;
    return memory_counter;
}

int sys_free_memory(uint32_t id) {
    return 0;
}

uint32_t* sys_list_files(uint32_t* count) {
    return fs_list_files(count);
}

// ============================================================
// 6. SHELL (commands, NOT system calls)
// ============================================================

void readline(char* buffer, int max_len) {
    int i = 0;
    while (1) {
        char c = keyboard_read_key();
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

void print_commands() {
    printk("============================================================\n");
    printk("COMMANDS\n");
    printk("============================================================\n");
    printk("\n");
    printk("  read <id>     - read file by ID\n");
    printk("  write <id> <text> - write to file\n");
    printk("  ls            - list files\n");
    printk("  run <name>    - create process\n");
    printk("  sleep <ms>    - sleep for ms\n");
    printk("  alloc <size>  - allocate memory\n");
    printk("  free <id>     - free memory\n");
    printk("  help          - this screen\n");
    printk("  about         - information about kernel\n");
    printk("  exit          - halt system\n");
    printk("\n");
    printk("============================================================\n");
    printk("  Basic syscalls: READ_FILE, WRITE_FILE, CREATE_PROCESS,\n");
    printk("  SLEEP, ALLOC_MEMORY, FREE_MEMORY, LIST_FILES\n");
    printk("------------------------------------------------------------\n");
    printk("  Additional calls (network, drivers, FS) must be loaded\n");
    printk("  as modules.\n");
    printk("============================================================\n");
}

void print_about() {
    printk("============================================================\n");
    printk("Manpupuner_42 v0.1\n");
    printk("============================================================\n");
    printk("  Idea & architecture author: Kaskov Aleksandr (Kafemin)\n");
    printk("  Technical consultant & code implementer: DeepSeek AI\n");
    printk("  Email: kafemin@gmail.com\n");
    printk("  License: MIT\n");
    printk("------------------------------------------------------------\n");
    printk("  Name origin:\n");
    printk("  Manpupuner plateau, Northern Urals.\n");
    printk("  7 stone pillars = 7 basic syscalls.\n");
    printk("  42 = universal answer.\n");
    printk("------------------------------------------------------------\n");
    printk("  7 basic syscalls:\n");
    printk("    READ_FILE, WRITE_FILE, CREATE_PROCESS, SLEEP,\n");
    printk("    ALLOC_MEMORY, FREE_MEMORY, LIST_FILES\n");
    printk("------------------------------------------------------------\n");
    printk("  Modular design: additional calls (network, drivers, FS)\n");
    printk("  are loaded as separate modules.\n");
    printk("============================================================\n");
}

void shell_start() {
    print_about();
    printk("\nType 'help' for commands\n\n");
    
    char cmd[256];
    while (1) {
        printk("> ");
        readline(cmd, 256);
        
        char* token = strtok(cmd, " ");
        if (!token) continue;
        
        if (strcmp(token, "help") == 0) {
            print_commands();
        }
        else if (strcmp(token, "about") == 0) {
            print_about();
        }
        else if (strcmp(token, "read") == 0) {
            char* arg = strtok(0, " ");
            if (!arg) { printk("Usage: read <id>\n"); continue; }
            uint32_t id = atoi(arg);
            char buffer[256];
            int size = sys_read_file(id, buffer, 255);
            if (size > 0) {
                buffer[size] = '\0';
                printk("%s\n", buffer);
            } else {
                printk("File not found\n");
            }
        }
        else if (strcmp(token, "write") == 0) {
            char* arg1 = strtok(0, " ");
            char* arg2 = strtok(0, "");
            if (!arg1 || !arg2) { printk("Usage: write <id> <text>\n"); continue; }
            uint32_t id = atoi(arg1);
            int len = 0;
            while (arg2[len]) len++;
            int result = sys_write_file(id, arg2, len);
            if (result >= 0) printk("Written %d bytes\n", result);
            else printk("File not found\n");
        }
        else if (strcmp(token, "ls") == 0) {
            uint32_t count;
            uint32_t* ids = sys_list_files(&count);
            printk("Files:\n");
            for (uint32_t i = 0; i < count; i++) {
                file_t* f = fs_get_file(ids[i]);
                printk("[%d] %s (%d bytes)\n", ids[i], f->name, f->size);
            }
        }
        else if (strcmp(token, "run") == 0) {
            char* arg = strtok(0, " ");
            if (!arg) { printk("Usage: run <name>\n"); continue; }
            uint32_t pid = sys_create_process(arg);
            printk("Process '%s' created, PID: %d\n", arg, pid);
        }
        else if (strcmp(token, "sleep") == 0) {
            char* arg = strtok(0, " ");
            if (!arg) { printk("Usage: sleep <ms>\n"); continue; }
            uint32_t ms = atoi(arg);
            sys_sleep(ms);
            printk("Slept %d ms\n", ms);
        }
        else if (strcmp(token, "alloc") == 0) {
            char* arg = strtok(0, " ");
            if (!arg) { printk("Usage: alloc <size>\n"); continue; }
            uint32_t size = atoi(arg);
            uint32_t id = sys_alloc_memory(size);
            printk("Allocated block ID: %d, size: %d\n", id, size);
        }
        else if (strcmp(token, "free") == 0) {
            char* arg = strtok(0, " ");
            if (!arg) { printk("Usage: free <id>\n"); continue; }
            uint32_t id = atoi(arg);
            sys_free_memory(id);
            printk("Freed block ID: %d\n", id);
        }
        else if (strcmp(token, "exit") == 0) {
            printk("Halting system...\n");
            while (1) { asm volatile("hlt"); }
        }
        else {
            printk("Unknown command: %s (type 'help' for list)\n", token);
        }
    }
}

// ============================================================
// 7. ENTRY POINT
// ============================================================

void kernel_main() {
    clear_screen();
    init_filesystem();
    shell_start();
}
