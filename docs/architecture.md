# Manpupuner_42 Architecture

## ⚠️ Disclaimer

This is a Proof of Concept. The kernel is experimental and has been tested **only** in the QEMU emulator. Running on real hardware is at your own risk.

**Kernel size: ~18-20 KB (v0.3-alpha).**

---

## Core Concept

**Manpupuner_42** is a hybrid arbiter kernel that provides only **7 basic system calls**. All other logic (file systems, drivers, network, graphics) is designed to be loaded modularly as separate components.

**Versions:**

| Version | Description | Size |
|:---|:---|:---|
| **v0.1** | Python prototype (PoC) | ~50 KB |
| **v0.3-alpha** | C/Assembly kernel with interrupts and multitasking | ~18-20 KB |

---

## Kernel Architecture Reference

To understand Manpupuner_42, it helps to know the landscape of kernel architectures. Here's how it compares to existing designs:

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
│  (temporarily disabled for stability)                       │
├─────────────────────────────────────────────────────────────┤
│                    ARBITER KERNEL (~18-20 KB)               │
│  ┌───────────┐  ┌────────────┐  ┌────────────────────┐    │
│  │ Syscalls  │  │ Scheduler  │  │ Memory Manager     │    │
│  │ (7 basic) │  │ (auto-    │  │ (alloc/free)       │    │
│  │           │  │  matic)   │  │                    │    │
│  └───────────┘  └────────────┘  └────────────────────┘    │
│  ┌───────────┐  ┌────────────┐  ┌────────────────────┐    │
│  │ IDT       │  │ VGA/UART  │  │ Context Switching  │    │
│  │ (stable)  │  │ (console) │  │ (assembly)         │    │
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
| **Unikernel** | Small footprint | ~18-20 KB |

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

## Kernel Components (v0.3-alpha)

| Component | Status | Description |
|:---|:---|:---|
| Bootloader (Multiboot) | ✅ | GRUB-compatible, x86 |
| VGA text output | ✅ | 80x25, 0xB8000 |
| UART (serial) | ✅ | COM1, 115200 baud |
| 7 system calls | ✅ | All implemented in C |
| IDT (interrupts) | ✅ | Stable, PIC remap |
| Timer (PIT) | ✅ | 100 Hz, process switching |
| Scheduler | ✅ | Automatic (timer-based) |
| Virtual filesystem | ✅ | In-memory, 16 files |
| Memory manager | ✅ | alloc/free, 32 blocks |
| Context switching | ✅ | Assembly |
| Shell | ✅ | Prompt `S> ` |
| Keyboard (PS/2) | ✅ | In kernel, Shift, Backspace |
| Modules (`load`) | ⚠️ | Loads, but `entry()` is disabled |
| Multitasking | ✅ | Dots `.` and `!` |

---

## Comparison with L4 / K42

| Aspect | L4 / K42 | Manpupuner_42 |
|:---|:---|:---|
| **Kernel size** | 12–50+ KB | **~18–20 KB** |
| **Syscall count** | 10+ (with IPC) | **Exactly 7** |
| **IPC** | Synchronous, fast (~5 µs) | Via files (IDs) |
| **Scheduler** | In-kernel | **Outside the kernel (module)** |
| **SMP / NUMA** | Yes | ❌ No |
| **Hot-swap modules** | Yes (K42) | ❌ No |
| **Linux compatibility** | Yes (L4Linux) | ❌ No (test processes only) |
| **Complexity** | High | **Low (low entry barrier)** |
| **Primary goal** | High-performance microkernel | **Proof of Concept / Principle demonstration** |

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

## Modules

All additional functionality is designed to be implemented as separate modules:

- **Keyboard module** (`keyboard_module.c`) — PS/2 keyboard support (loaded separately)
- **Network module** (planned) — TCP/IP stack
- **Graphics module** (planned) — VESA graphics
- **File system modules** (planned) — FAT32, ext2

---

## Design Principles

1. **Minimal kernel** — only 7 calls, ~18-20 KB
2. **Modular design** — everything else is separate
3. **Simple interface** — ID-based access
4. **Transparent debugging** — logging and traceability (UART)
5. **Cross-architecture inspiration** — best ideas from monolithic, microkernel, hybrid, exokernel, and unikernel
6. **Proof of Concept** — not a production kernel

---

## Project Structure

```
manpupuner_kernel/
├── boot.asm          # Bootloader (Multiboot). Context switch, IDT.
├── build.sh          # Build script: clean → compile → ISO.
├── docs/
│   └── architecture.md  # This file.
├── grub.cfg          # GRUB configuration.
├── kernel.c          # Kernel: 7 syscalls, VGA, UART, IDT, timer, FS, memory, shell.
├── LICENSE           # MIT License.
├── linker.ld         # Linker script.
├── Makefile          # Build: nasm + gcc + ld.
├── README.md         # Project description.
└── run.sh            # QEMU launcher with ISO.
```

---

## History

The idea was born in a discussion between Kaskov Aleksandr and DeepSeek AI. The goal was to prove that a kernel can be minimal and still support multiple ecosystems (POSIX, NT) through translation layers.

**Development stages:**

1. **v0.1 — Python PoC** — concept validation: [https://github.com/kafemin/Manpupuner_42](https://github.com/kafemin/Manpupuner_42)
2. **v0.3-alpha — C/Assembly kernel with interrupts and multitasking** — current stable version: [https://github.com/kafemin/manpupuner_kernel](https://github.com/kafemin/manpupuner_kernel)

> **Note:** The development of the C/Assembly kernel (v0.2-alpha) is now fully integrated into the v0.3-alpha release. The v0.3 branch is the main line of development and contains all features previously planned for v0.2.

---

## Real Hardware Warning

This kernel has **not** been tested on real hardware. The author has only tested it in the QEMU emulator. Use on real hardware at your own risk.

---

## Further Reading

- **Monolithic Kernel:** [Linux Kernel Documentation](https://www.kernel.org/doc/)
- **Microkernel:** [L4Re](https://l4re.org/)
- **Hybrid Kernel:** [Overview of Windows Architecture](https://learn.microsoft.com/ru-ru/windows-hardware/drivers/gettingstarted/overview-of-windows-architecture)
- **Exokernel:** [MIT Exokernel Operating System](https://pdos.csail.mit.edu/archive/exo/)
- **Unikernel:** [MirageOS](https://mirage.io/), [IncludeOS](https://www.includeos.org/)
