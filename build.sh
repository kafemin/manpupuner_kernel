#!/bin/bash
echo "=========================================="
echo "Building Manpupuner_42 Arbiter Kernel"
echo "=========================================="

rm -f *.o kernel.bin manpupuner.iso
rm -rf iso

cd src
nasm -f elf32 boot.asm -o boot.o
gcc -m32 -ffreestanding -nostdlib -c kernel.c -o kernel.o
gcc -m32 -ffreestanding -nostdlib -c keyboard_module.c -o keyboard.o
ld -m elf_i386 -T linker.ld -nostdlib boot.o kernel.o keyboard.o -o kernel.bin
mv kernel.bin ../
cd ..

mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
cat > iso/boot/grub/grub.cfg << EOF
set timeout=0
set default=0
menuentry "Manpupuner_42" {
    multiboot /boot/kernel.bin
    boot
}
EOF

grub-mkrescue -o manpupuner.iso iso/

echo "=========================================="
echo "Done! Image: manpupuner.iso"
echo "Run with: ./run.sh"
echo "=========================================="
