# Manpupuner_42

**Hybrid Arbiter Kernel — Proof of Concept**

[![Version](https://img.shields.io/badge/version-0.1-blue)](https://github.com/Kafemin/Manpupuner_42)
[![License](https://img.shields.io/badge/license-MIT-yellow)](LICENSE)

---

## ⚠️ DISCLAIMER

**This is a Proof of Concept / This is a demonstration.**

This kernel is experimental software. It may contain bugs, errors, or unexpected behavior. It has been tested **only** in the QEMU emulator. Running on real hardware is at your own risk. The author assumes no responsibility for any damage, data loss, or hardware failure that may occur.

**Kernel size: ~13 KB (less than 20 KB).**

---

## About

**Manpupuner_42** is a minimal hybrid arbiter kernel that provides only **7 basic system calls**. All other logic (file systems, drivers, network, graphics) must be loaded modularly as separate components.

The project name comes from the **Manpupuner plateau** in the Northern Urals. Seven stone pillars symbolize the 7 basic system calls. The number 42 refers to the search for a universal answer.

> *"People need to see the principle, and then... everything will take its course."*

---

## ⚠️ Known Limitations

- **Keyboard:** PS/2 driver works in QEMU. Real hardware may behave differently.
- **Filesystem:** Virtual in-memory only. No real disk support.
- **Graphics:** Text mode only (VGA).
- **Networking:** Not implemented.
- **Multitasking:** Not implemented.
- **Real hardware:** Untested. Use at your own risk.

---

## Repository

**GitHub:** [https://github.com/Kafemin/Manpupuner_42](https://github.com/Kafemin/Manpupuner_42)

**Original idea discussion:** [https://github.com/Kafemin/Manpupuner_42/discussions](https://github.com/Kafemin/Manpupuner_42/discussions)

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
git clone https://github.com/Kafemin/Manpupuner_42.git
cd Manpupuner_42

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

**Solution:** Use the keyboard module provided in `src/keyboard_module.c`. It handles PS/2 scancodes properly.

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
   - Arrow keys (print `[UP]`, `[DOWN]`, `[LEFT]`, `[RIGHT]`)

### ISO Size

The ISO is ~12 MB, but the kernel itself is only ~13 KB. The size comes from GRUB bootloader modules included in the ISO.

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
