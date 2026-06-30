#!/bin/bash

echo "=========================================="
echo "Running Manpupuner_42 v0.3-alpha (CPU limited)"
echo "=========================================="

taskset -c 0 qemu-system-x86_64 -cdrom manpupuner_42_v0.3.iso -m 256M -smp cores=1 -no-shutdown -monitor stdio
