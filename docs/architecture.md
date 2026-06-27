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

## Kernel Architecture Reference

Understanding Manpupuner_42 requires knowing the landscape of kernel architectures. Here's how it compares to existing designs:

### 1. Monolithic Kernel

**Examples:** Linux, BSD, Unix System V

**Characteristics:**
- All OS services run in kernel space
- Fast system calls (no context switching between components)
- Everything is tightly integrated
- Large kernel size (Linux: ~10-30 MB)

**Manpupuner_42 influence:** Direct syscall performance, speed-first approach

```
┌─────────────────────────────────────┐
│         USER SPACE                   │
│   Applications  │  Shell            │
├─────────────────────────────────────┤
│         KERNEL SPACE                 │
│  ┌──────────────────────────────┐   │
│  │  File System  │  Network     │   │
│  ├───────────────┼──────────────┤   │
│  │  Drivers      │  Scheduler   │   │
│  ├───────────────┼──────────────┤   │
│  │  Memory Mgmt  │  IPC         │   │
│  └──────────────────────────────┘   │
│         HARDWARE                    │
└─────────────────────────────────────┘
```

### 2. Microkernel

**Examples:** MINIX, L4, GNU Hurd, QNX, Mach

**Characteristics:**
- Minimal kernel core (IPC, scheduling, memory management)
- Drivers and services run as user-space processes
- Message passing between components
- Small kernel size (L4: ~10-20 KB)

**Manpupuner_42 influence:** Minimal core, modular drivers loaded separately

```
┌─────────────────────────────────────┐
│         USER SPACE                   │
│  Applications  │  Drivers  │  FS    │
│       ↕ IPC    │  ↕ IPC    │  ↕ IPC │
├─────────────────────────────────────┤
│         MICROKERNEL                  │
│  ┌──────────────────────────────┐   │
│  │   IPC  │  Scheduler          │   │
│  ├────────┼─────────────────────┤   │
│  │   Memory Mgmt  │  Syscalls   │   │
│  └──────────────────────────────┘   │
│         HARDWARE                    │
└─────────────────────────────────────┘
```

### 3. Hybrid Kernel

**Examples:** Windows NT, macOS/XNU, DragonFly BSD

**Characteristics:**
- Mix of monolithic and microkernel approaches
- Some services in kernel, some in user space
- Balance of performance and modularity
- Kernel size: ~1-10 MB

**Manpupuner_42 influence:** Balance between flexibility and speed, modular loading

```
┌─────────────────────────────────────┐
│         USER SPACE                   │
│  Apps  │  Services  │  Drivers      │
├─────────────────────────────────────┤
│         HYBRID KERNEL                │
│  ┌───────────────┐  ┌────────────┐  │
│  │   Core        │  │  Modules   │  │
│  │  Scheduler    │  │  FS        │  │
│  │  IPC          │  │  Network   │  │
│  │  Memory Mgmt  │  │  Graphics  │  │
│  └───────────────┘  └────────────┘  │
│         HARDWARE                    │
└─────────────────────────────────────┘
```

### 4. Exokernel

**Examples:** MIT Exokernel, Nemesis

**Characteristics:**
- Minimal abstraction of hardware
- Applications manage their own resources
- Library OS approach
- Tiny kernel core

**Manpupuner_42 influence:** Minimal abstraction, direct resource access (ID-based)

```
┌─────────────────────────────────────┐
│         USER SPACE                   │
│  App 1  │  App 2  │  LibOS  │  LibOS│
│    ↕    │    ↕    │    ↕    │    ↕   │
├─────────────────────────────────────┤
│         EXOKERNEL                    │
│  ┌──────────────────────────────┐   │
│  │   Secure HW Access            │   │
│  ├──────────────────────────────┤   │
│  │   Resource Management         │   │
│  └──────────────────────────────┘   │
│         HARDWARE                    │
└─────────────────────────────────────┘
```

### 5. Unikernel

**Examples:** MirageOS, IncludeOS, OSv

**Characteristics:**
- Single-address-space
- Library operating system
- Compiled for specific application
- Very small size (~1 MB)

**Manpupuner_42 influence:** Small footprint, single-purpose design philosophy

```
┌─────────────────────────────────────┐
│         UNIKERNEL                    │
│  ┌──────────────────────────────┐   │
│  │   Application + Kernel        │   │
│  │   (single address space)      │   │
│  └──────────────────────────────┘   │
│         HARDWARE                    │
└─────────────────────────────────────┘
```

---

## Manpupuner_42 Architecture

Manpupuner_42 is a **hybrid arbiter kernel** that combines elements from all major kernel architectures:

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATIONS / SHELL                     │
│  (User processes, shell commands, module execution)        │
├─────────────────────────────────────────────────────────────┤
│                    MODULES (optional)                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Keyboard │  Network │  Graphics │  File Systems    │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  Loaded dynamically via `load <module>` command             │
│  (entry() disabled in v0.2 for stability)                   │
├─────────────────────────────────────────────────────────────┤
│                    ARBITER KERNEL (~18-20 KB)               │
│  ┌───────────┐  ┌────────────┐  ┌────────────────────┐    │
│  │ Syscalls  │  │ Scheduler  │  │ Memory Manager     │    │
│  │ (7 basic) │  │ (manual    │  │ (alloc/free)       │    │
│  │           │  │  yield)    │  │                    │    │
│  └───────────┘  └────────────┘  └────────────────────┘    │
│  ┌───────────┐  ┌────────────┐  ┌────────────────────┐    │
│  │ FS Core   │  │ VGA/UART  │  │ Context Switching  │    │
│  │ (virtual) │  │ (console) │  │ (assembly)         │    │
│  └───────────┘  └────────────┘  └────────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│                    HARDWARE                                 │
│  ┌───────────┐  ┌────────────┐  ┌────────────────────┐    │
│  │ VGA       │  │ PS/2       │  │ UART (COM1)        │    │
│  │ 0xB8000   │  │ Keyboard   │  │ 115200 baud        │    │
│  └───────────┘  └────────────┘  └────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### How Manpupuner_42 Combines Kernel Architectures

| Architecture | Borrowed Concept | Implementation |
|:---|:---|:---|
| **Monolithic** | Performance, direct syscalls | Syscalls are direct C functions, no message passing overhead |
| **Microkernel** | Minimal core, modularity | Only 7 syscalls, drivers as separate modules |
| **Hybrid** | Balance of speed and flexibility | Core in kernel, optional modules loaded dynamically |
| **Exokernel** | Minimal abstraction | ID-based resource access, minimal layers |
| **Unikernel** | Small footprint | ~18-20 KB, single-purpose design |

---

## Why 7 Calls?

The number 7 is symbolic:
- 7 stone pillars on the Manpupuner plateau
- 7 basic operations cover 90% of system needs
- Minimalism reduces kernel size
- Easier to verify and secure

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
6. **Cross-architecture inspiration** — best ideas from monolithic, microkernel, hybrid, exokernel, and unikernel

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

---

## Further Reading

- **Monolithic Kernel:** [Linux Kernel Documentation](https://www.kernel.org/doc/)
- **Microkernel:** [MINIX 3](https://www.minix3.org/), [L4](https://l4hq.org/)
- **Hybrid Kernel:** [Windows NT Architecture](https://learn.microsoft.com/en-us/windows/win32/sysinfo/windows-kernel)
- **Exokernel:** [MIT Exokernel Paper](https://pdos.csail.mit.edu/archive/exo/)
- **Unikernel:** [MirageOS](https://mirage.io/), [IncludeOS](https://www.includeos.org/)
