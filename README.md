# Manpupuner_42

**Hybrid Arbiter Kernel — Proof of Concept**

[![Version](https://img.shields.io/github/v/release/kafemin/manpupuner_kernel)](https://github.com/kafemin/manpupuner_kernel/releases)
[![License](https://img.shields.io/badge/license-MIT-yellow)](LICENSE)

---

## ⚠️ DISCLAIMER

**This is a Proof of Concept / This is a demonstration.**

This kernel is experimental software. It may contain bugs, errors, or unexpected behavior. It has been tested **only** in the QEMU emulator. Running on real hardware is at your own risk. The author assumes no responsibility for any damage, data loss, or hardware failure that may occur.

**Additional limitations:**
- The kernel **does not have a shutdown command** (`shutdown`). To exit, use `Ctrl+C` in the terminal or QEMU monitor (`Ctrl+Alt+2` → `quit`).
- On real hardware, the following issues may occur:
  - Video mode initialization
  - PS/2 keyboard operation
  - Interrupt handling (APIC, ACPI)
  - Power management (ACPI is not implemented)

**Kernel size: ~18-20 KB.**

---

## About

**Manpupuner_42** is a minimal hybrid arbiter kernel that provides only **7 basic system calls**. All other logic (file systems, drivers, network, graphics) must be loaded modularly as separate components.

The project name comes from the **Manpupuner plateau** in the Northern Urals. Seven stone pillars symbolize the 7 basic system calls. The number 42 refers to the search for a universal answer.

> *"People need to see the principle, and then... everything will take its course."*

---

## 📦 Version History

### v0.1 — Concept Proof (Python)

- ✅ Python prototype demonstrating 7 syscalls
- ✅ POSIX and NT compatibility layers
- ✅ Virtual filesystem in memory
- ✅ Unit tests for all 7 syscalls
- ✅ Full documentation (RU/EN)

### v0.3-alpha — Kernel Implementation with Interrupts and Multitasking (CURRENT)

**Planned:**
- Stable interrupts (IDT)
- Automatic process switching (timer)
- Working modules (keyboard, drivers)
- Real filesystem (FAT32)
- ELF loader
- Full command documentation

**Implemented:**
- ✅ **IDT (Interrupt Descriptor Table)** — fully stable
- ✅ **PIC remap** (IRQ0 → vector 0x20)
- ✅ **Timer (PIT)** — 100 Hz
- ✅ **Interrupt handler** — prints `!` on every 10th tick
- ✅ **`sti`** — interrupts enabled!
- ✅ **Automatic process switching** — timer-based
- ✅ **Multitasking** — demonstrated by `.` and `!`
- ✅ **Shell** — working, prompt `S> `
- ✅ **Keyboard (PS/2)** — with Shift and Backspace
- ✅ **Virtual filesystem** (in-memory)
- ✅ **Memory manager** (alloc/free)

**Why not everything from the plan was implemented:**
During development, the project architecture was reconsidered. The main goal of v0.3 shifted from "implementing all planned features" to **proving the core mechanisms work**:
- Interrupts (IDT) — working ✅
- Multitasking — working ✅
- Stability — achieved ✅

FAT32, ELF loader and modules were moved to **v0.4**, as they are not critical for proving the concept.

---

## 📊 Manpupuner_42 vs L4 / K42 — Brief Comparison

| Aspect | L4 / K42 | Manpupuner_42 |
|:---|:---|:---|
| **Kernel size** | 12–50+ KB | **~18–20 KB** |
| **Syscall count** | 10+ (with IPC) | **Exactly 7** |
| **IPC** | Synchronous, fast (~5 µs) | Via files (IDs) |
| **Scheduler** | In-kernel | **Outside the kernel (module)** |
| **SMP / NUMA** | Yes | ❌ No |
| **Hot-swap modules** | Yes (K42) | ❌ No |
| **Linux compatibility** | Yes (L4Linux) | ❌ No (test processes only) |
| **Modules** | Services via IPC | External layers (POSIX/NT) |
| **Complexity** | High | **Low (low entry barrier)** |
| **Primary goal** | High-performance microkernel | **Proof of Concept / Principle demonstration** |

### 🔥 What makes Manpupuner_42 unique

- ✅ **Minimal API** — only 7 syscalls (instead of hundreds)
- ✅ **Scheduler outside the kernel** — replaceable without rebuilding
- ✅ **IPC via files** — unified interface
- ✅ **~18–20 KB** — one of the smallest kernels
- ✅ **Portable** — pure C + 200–300 lines of assembly

---

## ⚙️ Tested Environment

The kernel was developed and tested on the following setup:

| Component | Version |
|:---|:---|
| **OS** | Ubuntu 24.04.4 LTS (Noble) |
| **QEMU** | 8.2.2 |
| **NASM** | 2.16.01 |
| **GCC** | 13.3.0 |
| **LD** | 2.42 |
| **GRUB** | 2.12 |
| **Xorriso** | 1.5.6 |

```bash
lsb_release -a
No LSB modules are available.
Distributor ID:  Ubuntu
Description:     Ubuntu 24.04.4 LTS
Release:         24.04
Codename:        noble

qemu-system-x86_64 --version | head -1
QEMU emulator version 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.16)

nasm -v
NASM version 2.16.01

gcc --version | head -1
gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0

ld --version | head -1
GNU ld (GNU Binutils for Ubuntu) 2.42

grub-mkrescue --version | head -1
grub-mkrescue (GRUB) 2.12-1ubuntu7.3

xorriso --version | head -1
xorriso 1.5.6
```

---

## 🔥 Heating Issues and Solutions

When running the kernel in QEMU, the laptop may heat up due to high CPU usage. Below are the causes and how they are addressed in this project.

### Causes

1. **No hardware acceleration** (KVM/HVF) — QEMU emulates every instruction via TCG (software emulation), which loads the CPU up to 100%.

2. **Full CPU core usage** — QEMU uses all available cores by default.

3. **Uninterrupted idle loop** — the bare-metal kernel does not have ACPI or HLT-optimized idle states.

### Solutions Implemented in This Project

| Solution | How it's done |
|:---|:---|
| **Limit CPU cores** | `-smp cores=1` restricts QEMU to one core |
| **Limit CPU usage** | `cpulimit -p $PID -l 50` caps CPU at 50% |
| **CPU affinity** | `taskset -c 0` binds QEMU to a single core |
| **Fast termination** | Run with `-no-shutdown -monitor stdio` and exit via `Ctrl+C` or QEMU monitor |
| **Shutdown port** | (Optional) The kernel can send `outw(0x2000, 0x0604)` to shut down QEMU |

### Example `run.sh` with CPU Limiting

```bash
#!/bin/bash

echo "=========================================="
echo "Running Manpupuner_42 v0.3-alpha (CPU limited)"
echo "=========================================="

# Start QEMU in background with 1 core
qemu-system-x86_64 -cdrom manpupuner_42_v0.3.iso -m 256M -smp cores=1 -no-shutdown -monitor stdio &

sleep 1
PID=$(pgrep -f qemu-system-x86_64)

if [ -n "$PID" ]; then
    echo "QEMU PID: $PID"
    echo "Limiting CPU to 50%..."
    cpulimit -p $PID -l 50
else
    echo "QEMU not found. Check if it's running."
fi
```

### Additional Tips

- Always close QEMU via `Ctrl+C` or the monitor (`Ctrl+Alt+2` → `quit`).
- Use `htop` to check CPU usage after each session.
- If QEMU hangs in the background, kill it with:
  ```bash
  pkill -9 -f qemu-system-x86_64
  ```

---

## ⚠️ Known Issues (v0.3-alpha)

| Issue | Description |
|:---|:---|
| **`load <module>`** | Loading a module with `entry()` enabled causes a system reboot. Temporarily disabled. |
| **Arrow keys (↑ ↓ ← →)** | Not supported. Do not work. |
| **Real hardware** | Untested. Use only in QEMU. |
| **`shutdown` command** | Not implemented. Exit via `Ctrl+C` or QEMU monitor. |

---

## 7 Basic System Calls

| # | Syscall | Description |
|:-:|:---|:---|
| 1 | `READ_FILE` | Read data by ID |
| 2 | `WRITE_FILE` | Write data by ID |
| 3 | `CREATE_PROCESS` | Create a new process |
| 4 | `SLEEP` | Sleep for milliseconds |
| 5 | `ALLOC_MEMORY` | Allocate memory block |
| 6 | `FREE_MEMORY` | Free memory block |
| 7 | `LIST_FILES` | List all file IDs |

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   APPLICATIONS / SHELL                  │
├─────────────────────────────────────────────────────────┤
│               KEYBOARD MODULE (PS/2)                    │
│              (separate component)                       │
├─────────────────────────────────────────────────────────┤
│                    ARBITER KERNEL                       │
│                  7 basic syscalls                       │
│                                                         │
│   ┌───────────────┐  ┌──────────────┐  ┌────────────┐ │
│   │   Microkernel  │  │   Monolithic │  │   Hybrid   │ │
│   │   (MINIX, L4)  │  │   (Linux)    │  │  (Windows, │ │
│   │                │  │              │  │   macOS)   │ │
│   └───────────────┘  └──────────────┘  └────────────┘ │
│                                                         │
│   Manpupuner_42 combines ideas from all three:         │
│   • Microkernel — minimal core, modular drivers        │
│   • Monolithic — speed, direct syscalls                │
│   • Hybrid — balance between flexibility & speed       │
├─────────────────────────────────────────────────────────┤
│              HARDWARE (VGA, PS/2, UART)                │
└─────────────────────────────────────────────────────────┘
```

### Kernel Architecture Comparison

| Architecture | Examples | Manpupuner_42 Influence |
|:---|:---|:---|
| **Microkernel** | MINIX, L4, GNU Hurd | Minimal core, drivers as modules |
| **Monolithic** | Linux, BSD | Fast syscalls, direct hardware access |
| **Hybrid** | Windows NT, macOS/XNU | Balance of speed and modularity |
| **Exokernel** | MIT Exokernel | Minimal abstraction, direct resource access |
| **Unikernel** | MirageOS, IncludeOS | Single-address-space, library OS |

**Manpupuner_42** is a **hybrid arbiter kernel** that takes:
- **Microkernel philosophy**: Keep the core minimal (only 7 syscalls), move everything else to modules
- **Monolithic performance**: Direct syscalls without message passing overhead
- **Hybrid flexibility**: Optional module loading for extended functionality
- **Exokernel ideas**: Applications can manage their own resources
- **Unikernel simplicity**: Small footprint, single-purpose design

---

## Quick Start (QEMU — Recommended)

```bash
# Clone
git clone https://github.com/kafemin/manpupuner_kernel.git
cd manpupuner_kernel

# Build
chmod +x build.sh run.sh
./build.sh

# Run in QEMU
./run.sh
```

---

## Commands (Shell)

| Command | Description |
|:---|:---|
| `help` | Show commands |
| `version` | Show kernel version |
| `syscalls` | List 7 system calls |
| `ls` | List files |
| `read <id>` | Read file by ID |
| `write <id> <text>` | Write to file |
| `alloc <size>` | Allocate memory |
| `free <id>` | Free memory |
| `run <name>` | Create a process |

---

## Build Requirements

- `nasm` — assembler
- `gcc` (i386-elf or multilib)
- `ld` (i386)
- `grub-mkrescue`
- `xorriso`
- `qemu-system-x86` (for testing)

**Ubuntu/Debian installation:**

```bash
sudo apt install nasm gcc gcc-multilib grub-pc-bin grub-common xorriso qemu-system-x86
```

---

## Project Structure

```
manpupuner_kernel/
├── boot.asm          # Bootloader (Multiboot). Context switch, IDT, interrupt handler.
├── build.sh          # Build script: clean → compile → create ISO.
├── docs/
│   └── architecture.md  # Architecture documentation.
├── grub.cfg          # GRUB configuration for kernel boot.
├── kernel.c          # Kernel: 7 syscalls, VGA, UART, IDT, timer, scheduler, FS, memory, shell.
├── LICENSE           # MIT License.
├── linker.ld         # Linker script — defines memory section layout.
├── Makefile          # Build: nasm + gcc + ld.
├── README.md         # Project description, versions, commands, build guide.
└── run.sh            # QEMU launcher with ISO and CPU limit.
```

---

## Plans for v0.4

**Main goal:** Prove that POSIX and NT calls can be executed simultaneously on a single kernel.

| Task | Description |
|:---|:---|
| **External scheduler** | Scheduler as a module outside the kernel |
| **External FS** | Filesystem outside the kernel (in-memory) |
| **POSIX layer** | POSIX call emulation (`read`, `write`, `open`) |
| **NT layer** | NT call emulation (`NtReadFile`, `NtWriteFile`) |
| **Syscall delegation** | `sys_read_file()` delegates to the scheduler |
| **Test** | POSIX and NT processes read the same file alternately |

> **Important:** The kernel stays minimal (7 syscalls) and knows nothing about layers. All logic is in the scheduler and modules.

---

## Real Hardware (Untested)

**⚠️ Running on real hardware is at your own risk!**

If you want to try it on real hardware:

1. Write the ISO to a USB drive:
   ```bash
   sudo dd if=manpupuner.iso of=/dev/sdX bs=4M status=progress
   ```
   (Replace `/dev/sdX` with your actual USB device!)

2. Boot from the USB drive.

3. The system should start. However, keyboard and video behavior may differ.

---

## History

The idea was born in a discussion between Kaskov Aleksandr and DeepSeek AI. The goal was to prove that a kernel can be minimal and still support multiple ecosystems (POSIX, NT) through translation layers.

**Development stages:**

1. **v0.1 — Python PoC** — concept validation: [https://github.com/kafemin/Manpupuner_42](https://github.com/kafemin/Manpupuner_42)
2. **v0.3-alpha — C/Assembly kernel with interrupts and multitasking** — current stable version: [https://github.com/kafemin/manpupuner_kernel](https://github.com/kafemin/manpupuner_kernel)

> **Note:** The development of the C/Assembly kernel (v0.2-alpha) is now fully integrated into the v0.3-alpha release. The v0.3 branch is the main line of development and contains all features previously planned for v0.2.

---

## License

MIT © 2026 Kaskov Aleksandr

---

## Authors

| Role | Name |
|:---|:---|
| Idea & Architecture | **Kaskov Aleksandr** (Kafemin) |
| Technical Consultant & Code Implementation | DeepSeek AI |

---

> *"People need to see the principle, and then... everything will take its course."*
