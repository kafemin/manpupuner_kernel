# Manpupuner_42 Architecture

## ⚠️ Disclaimer

This is a Proof of Concept. The kernel is experimental and has been tested only in the QEMU emulator. Running on real hardware is at your own risk.

**Kernel size: ~18-20 KB (v0.2-alpha).**

---

## Core Concept

Manpupuner_42 is a **hybrid arbiter kernel** that provides only **7 basic system calls**. All other logic (file systems, drivers, network, graphics) is designed to be loaded modularly as separate components.

**Version History:**

| Version | Description | Size |
|:---|:---|:---|
| **v0.1** | Python prototype (PoC) | ~50 KB |
| **v0.2-alpha** | C/Assembly kernel | ~18-20 KB |
| **v0.3** | Planned: stable interrupts, working modules | ~25 KB |

---

## Why 7 Calls?

The number 7 is symbolic:
- 7 stone pillars on the Manpupuner plateau
- 7 basic operations cover 90% of system needs
- Minimalism reduces kernel size

## System Calls

| # | Call | Description | Parameters |
|:-:|:---|:---|:---|
| 1 | READ_FILE | Read data by ID | `id`, `buffer`, `size` |
| 2 | WRITE_FILE | Write data by ID | `id`, `data`, `size` |
| 3 | CREATE_PROCESS | Create a new process | `name` |
| 4 | SLEEP | Sleep for milliseconds | `milliseconds` |
| 5 | ALLOC_MEMORY | Allocate memory | `size` |
| 6 | FREE_MEMORY | Free memory | `id` |
| 7 | LIST_FILES | List all file IDs | `count` |

## Kernel Components (v0.2-alpha)

| Component | Status | Description |
|:---|:---|:---|
| Bootloader (Multiboot) | ✅ | GRUB-compatible, x86 |
| VGA text output | ✅ | 80x25, 0xB8000 |
| UART (serial) | ✅ | COM1, 115200 baud |
| 7 system calls | ✅ | All implemented in C |
| Virtual filesystem | ✅ | In-memory, 16 files |
| Memory manager | ✅ | alloc/free, 32 blocks |
| Process scheduler | ✅ | Manual yield, 16 processes |
| Context switching | ✅ | Assembly (pusha/popa) |
| Emergency shell | ✅ | reboot, dump, load, help, version, syscalls |
| Module loader | ⚠️ | Loads modules, but entry() is disabled (causes reboot) |
| Keyboard driver | ✅ | PS/2, in kernel (temporary) |
| Backspace support | ✅ | Works |
| Shift support | ✅ | Uppercase, _, ? |
| Timer (PIT) | ⚠️ | In progress |
| IDT (interrupts) | ⚠️ | In progress |
| Automatic process switching | ❌ | Planned for v0.3 |
| Arrow keys | ❌ | Not supported |

## Known Limitations

| Issue | Status |
|:---|:---|
| `load <module>` with entry() enabled | ❌ Causes reboot (disabled for safety) |
| Arrow keys (↑ ↓ ← →) | ❌ Not supported |
| Some keyboard keys (laptop) | ⚠️ May not work correctly |
| IDT / interrupts | ⚠️ Not stable |
| Automatic multitasking | ❌ Not implemented |
| Real hardware | ⚠️ Untested |

## Modules

All additional functionality is designed to be implemented as separate modules:

- **Keyboard module** (`keyboard_module.c`) — PS/2 keyboard support (loaded separately)
- **Network module** (planned) — TCP/IP stack
- **Graphics module** (planned) — VESA graphics
- **File system modules** (planned) — FAT32, ext2

## Design Principles

1. **Minimal kernel** — only 7 calls, ~18-20 KB
2. **Modular design** — everything else is separate
3. **Simple interface** — ID-based access
4. **Transparent debugging** — logging and traceability (UART)
5. **Isolation** — contexts for different layers

## Project Structure

```
manpupuner_kernel/
├── boot.asm           # Bootloader (Multiboot)
├── build.sh           # ISO build script
├── docs/
│   └── architecture.md # This file
├── .gitignore         # Ignored files
├── kernel.c           # Kernel (7 syscalls, VGA, UART, FS, memory, processes, shell)
├── LICENSE            # MIT License
├── linker.ld          # Linker script for kernel
├── Makefile           # Build system
├── modules/
│   ├── keyboard_module.c # Keyboard module
│   └── module.ld      # Linker script for modules
├── README.md          # Project description
└── run.sh             # QEMU launcher
```

## History

The idea was born in a discussion between Kaskov Aleksandr and DeepSeek AI. The goal was to prove that a kernel can be minimal and still support multiple ecosystems (POSIX, NT) through translation layers.

**Two-stage development:**

1. **v0.1 — Python PoC** — concept validation: [https://github.com/kafemin/Manpupuner_42](https://github.com/kafemin/Manpupuner_42)
2. **v0.2-alpha — C/Assembly kernel** — implementation: [https://github.com/kafemin/manpupuner_kernel](https://github.com/kafemin/manpupuner_kernel)

> *"People need to see the principle, and then... everything will take its course."*

## Real Hardware Warning

This kernel has **not** been tested on real hardware. The author has only tested it in the QEMU emulator. Use on real hardware at your own risk.
