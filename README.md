# Manpupuner_42

**Hybrid Arbiter Kernel — Proof of Concept**

[![Version](https://img.shields.io/badge/version-0.2-alpha-blue)](https://github.com/kafemin/manpupuner_kernel)
[![License](https://img.shields.io/badge/license-MIT-yellow)](LICENSE)

---

## ⚠️ DISCLAIMER

**This is a Proof of Concept / This is a demonstration.**

This kernel is experimental software. It may contain bugs, errors, or unexpected behavior. It has been tested **only** in the QEMU emulator. Running on real hardware is at your own risk. The author assumes no responsibility for any damage, data loss, or hardware failure that may occur.

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

### v0.2-alpha — Kernel Implementation (C/Assembly)

- ✅ Bootloader (Multiboot, GRUB)
- ✅ VGA text mode output
- ✅ UART (serial port) for debugging
- ✅ 7 system calls implemented in C
- ✅ Virtual filesystem (in-memory)
- ✅ Memory manager (alloc/free)
- ✅ Process scheduler (manual yield)
- ✅ Context switching (assembly)
- ✅ Emergency shell (reboot, dump, load, help, version, syscalls)
- ✅ Module loader (load keyboard.bin — entry disabled)
- ✅ Keyboard driver (PS/2) in kernel
- ✅ Backspace support
- ✅ Shift support (uppercase, _, ?)

### v0.3 — Planned

- ⏳ Stable interrupts (IDT)
- ⏳ Automatic process switching (timer)
- ⏳ Working modules (keyboard, drivers)
- ⏳ Real filesystem (FAT32)
- ⏳ ELF loader
- ⏳ Full command documentation

---

## ⚠️ Known Issues (v0.2-alpha)

| Issue | Description |
|:---|:---|
| **`load <module>`** | Loading a module with `entry()` enabled causes a system reboot. For safety, `entry()` is currently disabled in `load_module()`. |
| **Module not found** | Works correctly — prints "Module file not found". No reboot. |
| **Arrow keys (↑ ↓ ← →)** | Not supported. Do not work. |
| **Keyboard keys** | Some keys may not work correctly (tested on a laptop keyboard). QEMU emulation may behave differently. |
| **Interrupts (IDT)** | Not yet stable. Disabled by default. |
| **Automatic multitasking** | Not implemented. Only manual `yield` is available. |
| **Real hardware** | Untested. Use only in QEMU. |

---

## Repository

**GitHub:** [https://github.com/kafemin/manpupuner_kernel](https://github.com/kafemin/manpupuner_kernel)

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
┌─────────────────────────────────────────┐
│           APPLICATIONS / SHELL           │
├─────────────────────────────────────────┤
│          KEYBOARD MODULE (PS/2)          │
│         (separate component)             │
├─────────────────────────────────────────┤
│              ARBITER KERNEL              │
│            7 basic syscalls              │
├─────────────────────────────────────────┤
│              HARDWARE (VGA, PS/2)        │
└─────────────────────────────────────────┘
```

---

## Quick Start (QEMU — Recommended)

```bash
# Clone
git clone https://github.com/kafemin/manpupuner_kernel.git
cd manpupuner_kernel

# Build
chmod +x build.sh run.sh
./build.sh

# Run in QEMU (RECOMMENDED)
./run.sh
```

---

## Commands (Shell)

| Command | Description |
|:---|:---|
| `help` | Show commands |
| `about` | Show kernel info |
| `ls` | List files |
| `read <id>` | Read file by ID |
| `write <id> <text>` | Write to file |
| `run <name>` | Create a process |
| `sleep <ms>` | Sleep for ms |
| `alloc <size>` | Allocate memory |
| `free <id>` | Free memory |
| `exit` | Halt system |

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

## Known Issues

### Keyboard in QEMU

In QEMU, the keyboard may not work correctly with the default PS/2 emulation. This is a known issue with QEMU's PS/2 emulation.

**Solution:** Use the keyboard module provided in `modules/keyboard_module.c`. It handles PS/2 scancodes properly.

**If the keyboard still doesn't work:**

1. Try running without USB options:
   ```bash
   qemu-system-x86_64 -cdrom manpupuner.iso -m 256M
   ```

2. The keyboard module supports:
   - All letters and numbers
   - Spacebar
   - Backspace
   - Enter
   - Shift + letters (uppercase)
   - Shift + '-' = '_'
   - Shift + '/' = '?'

**Note:** Arrow keys (↑ ↓ ← →) are **not supported** in the current version.

### ISO Size

The ISO is ~12 MB, but the kernel itself is only ~18-20 KB. The size comes from GRUB bootloader modules included in the ISO.

---

## Project Structure

```
manpupuner_kernel/
├── boot.asm           # Bootloader (Multiboot)
├── build.sh           # ISO build script
├── docs/
│   └── architecture.md # Architecture documentation
├── .gitignore         # Ignored files
├── kernel.c           # Kernel (7 syscalls, VGA, UART, FS, memory, processes, shell)
├── LICENSE            # MIT License
├── linker.ld          # Linker script for kernel
├── Makefile           # Build system
├── modules/
│   ├── keyboard_module.c # Keyboard module
│   └── module.ld      # Linker script for modules
├── README.md          # This file
└── run.sh             # QEMU launcher
```

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
