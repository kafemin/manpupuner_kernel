#!/bin/bash
# run.sh — запуск Manpupuner_42 v0.2-engine в QEMU
# Запуск: ./run.sh

echo "=========================================="
echo "Running Manpupuner_42 v0.2-engine in QEMU"
echo "=========================================="

qemu-system-x86_64 -cdrom manpupuner_42_v0.2.iso -m 256M
