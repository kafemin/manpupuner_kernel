; boot.asm — загрузчик Manpupuner_42 (Multiboot)
; Архитектура: x86 (32-bit)
; Сборка: nasm -f elf32 boot.asm -o boot.o

bits 32

; ============================================================
; Multiboot-заголовок (для GRUB)
; ============================================================

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

; ============================================================
; Точка входа
; ============================================================

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

; ============================================================
; Переключение контекста
; ============================================================

global switch_context
switch_context:
    pusha
    push ds
    push es
    push fs
    push gs

    mov eax, [esp + 36]
    mov [eax], esp

    mov eax, [esp + 40]
    mov ecx, [esp]
    mov [eax], ecx

    mov eax, [esp + 44]
    mov esp, [eax]

    mov eax, [esp + 48]
    mov ebx, [eax]

    pop gs
    pop fs
    pop es
    pop ds
    popa

    jmp ebx

; ============================================================
; Стек
; ============================================================

section .bss
align 16
    resb 16384
stack_top:
