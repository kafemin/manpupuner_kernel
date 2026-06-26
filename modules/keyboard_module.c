// keyboard_module.c — модуль клавиатуры для Manpupuner_42
#include <stdint.h>

#define MODULE_MAGIC 0x4D504B32
#define KEYBOARD_PORT 0x60
#define KEYBOARD_STATUS 0x64
#define DEV_KEYBOARD 100

struct module_header {
    uint32_t magic;
    uint32_t entry_point;
    uint32_t data_size;
};

// Системные вызовы (заполняются ядром при загрузке)
static int (*sys_write_file)(uint32_t, const char*, uint32_t) = 0;
static int (*sys_read_file)(uint32_t, char*, uint32_t) = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static const char scancode_table[] = {
    0,   0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
};

void module_init(int (*write)(uint32_t, const char*, uint32_t),
                 int (*read)(uint32_t, char*, uint32_t)) {
    sys_write_file = write;
    sys_read_file = read;
}

void module_main() {
    if (!sys_write_file || !sys_read_file) return;
    
    while (1) {
        if ((inb(KEYBOARD_STATUS) & 1) == 0) continue;
        uint8_t scancode = inb(KEYBOARD_PORT);
        if (scancode & 0x80) continue;
        
        char c = 0;
        if (scancode == 0x39) c = ' ';
        else if (scancode == 0x1C) c = '\n';
        else if (scancode == 0x0E) c = '\b';
        else if (scancode < sizeof(scancode_table)) {
            c = scancode_table[scancode];
        }
        
        if (c) {
            sys_write_file(DEV_KEYBOARD, &c, 1);
        }
    }
}

__attribute__((section(".module")))
struct module_header header = {
    .magic = MODULE_MAGIC,
    .entry_point = (uint32_t)&module_main,
    .data_size = 0
};
