/* linker.ld — скрипт линковки для ядра Manpupuner_42 */
/* Архитектура: x86 (32-bit) */
/* Сборка: ld -m elf_i386 -T linker.ld -nostdlib boot.o kernel.o -o kernel.bin */

ENTRY(start)

SECTIONS {
    /* Ядро загружается по адресу 1 МБ (стандарт Multiboot) */
    . = 1M;

    /* Секция кода */
    .text : {
        *(.multiboot)
        *(.text)
    }

    /* Секция данных */
    .data : {
        *(.data)
    }

    /* Секция BSS (неинициализированные данные) */
    .bss : {
        *(.bss)
    }

    /* Выравнивание по границе 4 КБ */
    . = ALIGN(4096);
}
