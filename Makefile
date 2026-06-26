# Makefile — сборка ядра Manpupuner_42 (v0.2-engine)

ASM = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
RM = rm -f

ASMFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

OBJS = boot.o kernel.o module_data.o
TARGET = kernel.bin

MODULE_DIR = modules
MODULE_BIN = $(MODULE_DIR)/keyboard_module.bin
MODULE_OBJ = module_data.o

.PHONY: all clean modules

all: $(TARGET)

# ============================================================
# Ядро
# ============================================================

boot.o: boot.asm
	$(ASM) $(ASMFLAGS) boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Встраиваем модуль как объектный файл
$(MODULE_OBJ): $(MODULE_BIN)
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 $(MODULE_BIN) $(MODULE_OBJ)

$(TARGET): $(OBJS) $(MODULE_OBJ)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

# ============================================================
# Модуль клавиатуры
# ============================================================

modules:
	mkdir -p $(MODULE_DIR)

# Для модуля клавиатуры
$(MODULE_BIN): modules
	cd $(MODULE_DIR) && \
	gcc -m32 -ffreestanding -nostdlib -c keyboard_module.c -o keyboard_module.o && \
	ld -m elf_i386 -T module.ld -nostdlib keyboard_module.o -o keyboard_module.elf && \
	objcopy -O binary keyboard_module.elf keyboard_module.bin

# ============================================================
# Очистка
# ============================================================

clean:
	$(RM) $(OBJS) $(TARGET) $(MODULE_OBJ)
	$(RM) $(MODULE_DIR)/*.o $(MODULE_DIR)/*.bin
