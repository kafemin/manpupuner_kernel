// keyboard_module.c — модуль клавиатуры (PS/2)
// Компилируется отдельно и линкуется с ядром

#include <stdint.h>
#include <stddef.h>

#define KEYBOARD_PORT 0x60
#define KEYBOARD_STATUS 0x64

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

static char last_key = 0;
static int shift_pressed = 0;

int keyboard_available() {
    return (inb(KEYBOARD_STATUS) & 1) != 0;
}

char keyboard_read_key() {
    while (1) {
        if (!keyboard_available()) continue;
        uint8_t scancode = inb(KEYBOARD_PORT);
        if (scancode & 0x80) {
            // Отпускание Shift
            if (scancode == 0xAA) shift_pressed = 0;
            if (scancode == 0xB6) shift_pressed = 0;
            continue;
        }
        // Нажатие Shift
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            continue;
        }
        // Обработка расширенных скан-кодов (0xE0) — стрелки
        if (scancode == 0xE0) {
            while (!keyboard_available()) continue;
            uint8_t b2 = inb(KEYBOARD_PORT);
            if (b2 == 0x48) return 0x80;  // UP
            if (b2 == 0x50) return 0x81;  // DOWN
            if (b2 == 0x4B) return 0x82;  // LEFT
            if (b2 == 0x4D) return 0x83;  // RIGHT
            continue;
        }

        // Пробел
        if (scancode == 0x39) return ' ';
        // Enter
        if (scancode == 0x1C) return '\n';
        // Backspace
        if (scancode == 0x0E) return '\b';

        if (scancode < sizeof(scancode_table)) {
            char c = scancode_table[scancode];
            if (c) {
                // Если нажат Shift — делаем заглавную
                if (shift_pressed && c >= 'a' && c <= 'z') {
                    return c - ('a' - 'A');
                }
                return c;
            }
        }
    }
}
