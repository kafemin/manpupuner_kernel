/* linker.ld — скрипт линковки для ядра Manpupuner_42 */
ENTRY(start)

SECTIONS {
    . = 1M;

    .text : {
        *(.multiboot)
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        . = ALIGN(16);
        *(.bss)
    }

    . = ALIGN(4096);
}
