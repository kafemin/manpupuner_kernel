#!/bin/bash

echo "=========================================="
echo "Building Manpupuner_42 v0.3-alpha"
echo "=========================================="

echo "[1/4] Cleaning old builds..."
make clean

# Remove old ISO images and build artifacts
rm -f manpupuner_42_v*.iso
rm -rf iso/

echo "[2/4] Building kernel..."
make

echo "[3/4] Preparing ISO structure..."
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
cp grub.cfg iso/boot/grub/

echo "[4/4] Creating ISO image..."
grub-mkrescue -o manpupuner_42_v0.3.iso iso/

echo "=========================================="
echo "Done! Image: manpupuner_42_v0.3.iso"
echo "Run with: ./run.sh"
echo "=========================================="
