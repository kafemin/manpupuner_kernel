bits 32
section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .text
global start
extern kernel_main

start:
    mov esp, stack_top
    push eax
    push ebx
    call kernel_main
    cli
    hlt

section .bss
align 16
    resb 16384
stack_top:
