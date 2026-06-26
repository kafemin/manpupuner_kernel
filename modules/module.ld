/* module.ld — скрипт линковки для модуля клавиатуры */
ENTRY(module_main)

SECTIONS {
    . = 0x0;
    .module : {
        *(.module)
    }
    .text : {
        *(.text)
    }
    .data : {
        *(.data)
    }
    .bss : {
        *(.bss)
    }
}
