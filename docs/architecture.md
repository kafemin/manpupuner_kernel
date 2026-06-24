# Manpupuner_42 Architecture

## ⚠️ Disclaimer

This is a Proof of Concept. The kernel is experimental and has been tested only in the QEMU emulator. Running on real hardware is at your own risk.

**Kernel size: ~13 KB (less than 20 KB).**

---

## Core Concept

Manpupuner_42 is a **hybrid arbiter kernel** that provides only 7 basic system calls. All other logic (file systems, drivers, network, graphics) is designed to be loaded modularly as separate components.

## Why 7 Calls?

The number 7 is symbolic:
- 7 stone pillars on the Manpupuner plateau
- 7 basic operations cover 90% of system needs
- Minimalism reduces kernel size to ~13 KB

## System Calls

| Call | Description | Parameters |
|:---|:---|:---|
| READ_FILE | Read data by ID | `id`, `buffer`, `size` |
| WRITE_FILE | Write data by ID | `id`, `data`, `size` |
| CREATE_PROCESS | Create a new process | `name` |
| SLEEP | Sleep for milliseconds | `milliseconds` |
| ALLOC_MEMORY | Allocate memory | `size` |
| FREE_MEMORY | Free memory | `id` |
| LIST_FILES | List all file IDs | `count` |

## Modules

All additional functionality must be implemented as separate modules:

- **Keyboard module** (`keyboard_module.c`) — PS/2 keyboard support
- **Network module** (planned) — TCP/IP stack
- **Graphics module** (planned) — VGA graphics
- **File system modules** (planned) — ext2, FAT, NTFS support

## Design Principles

1. **Minimal kernel** — only 7 calls, ~13 KB
2. **Modular design** — everything else is separate
3. **Simple interface** — ID-based access
4. **Transparent debugging** — logging and traceability
5. **Isolation** — contexts for different layers

## History

The idea was born in a discussion between Kaskov Aleksandr and DeepSeek AI. The goal was to prove that a kernel can be minimal and still support multiple ecosystems (POSIX, NT) through translation layers.

> *"People need to see the principle, and then... everything will take its course."*

## Real Hardware Warning

This kernel has **not** been tested on real hardware. The author has only tested it in the QEMU emulator. Use on real hardware at your own risk.
