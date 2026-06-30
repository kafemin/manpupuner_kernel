; boot.asm — Manpupuner_42 Multiboot loader
bits 32

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .text
global start
extern kernel_main
extern irq_handler_c

start:
    mov esp, stack_top
    push eax
    push ebx
    call kernel_main
    cli
    hlt

global switch_context
switch_context:
    mov eax, [esp + 4]
    mov ecx, [esp + 12]
    push ebp
    push ebx
    push esi
    push edi
    mov [eax], esp
    mov esp, ecx
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

global load_idt
load_idt:
    lidt [idt_desc]
    ret

global reload_idt
reload_idt:
    lidt [idt_desc]
    ret

global irq_handler
irq_handler:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    call irq_handler_c
    mov al, 0x20
    out 0x20, al
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

extern idt_desc

section .bss
align 16
    resb 16384
stack_top:
