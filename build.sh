#!/bin/bash
# build.sh — сборка ISO-образа Manpupuner_42 (v0.2-engine)
# Запуск: ./build.sh

set -e

echo "=========================================="
echo "Building Manpupuner_42 v0.2-engine"
echo "=========================================="

# 1. Сборка ядра
echo "[1/3] Building kernel..."
make clean
make

# 2. Подготовка ISO
echo "[2/3] Preparing ISO structure..."
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/

# 3. Создание конфига GRUB
cat > iso/boot/grub/grub.cfg << EOF
set timeout=0
set default=0
menuentry "Manpupuner_42 v0.2-engine" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# 4. Сборка ISO
echo "[3/3] Creating ISO image..."
grub-mkrescue -o manpupuner_42_v0.2.iso iso/

echo "=========================================="
echo "Done! Image: manpupuner_42_v0.2.iso"
echo "Run with: ./run.sh"
echo "=========================================="
