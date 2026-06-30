# Makefile — Manpupuner_42 v0.3-alpha

ASM = nasm
CC = gcc
LD = ld
RM = rm -f

ASMFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -Iinclude -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

OBJS = boot.o kernel.o
TARGET = kernel.bin

.PHONY: all clean

all: $(TARGET)

boot.o: boot.asm
	$(ASM) $(ASMFLAGS) boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET)
