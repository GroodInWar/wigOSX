<!--
@file README.md
@brief Project overview for wigOSX.
-->

# wigOSX

wigOSX is a small freestanding i386 kernel project. The repository is arranged
so generic kernel code, hardware drivers, architecture-specific code, boot
inputs, and generated artifacts are easy to tell apart.

## Project Layout

```text
.
├── boot/                 # Multiboot entry assembly
├── include/kernel/       # Public kernel headers
│   ├── arch/i386/        # i386 CPU, GDT, IDT, I/O, and PIC headers
│   ├── core/             # Generic kernel interfaces
│   └── drivers/          # Driver interfaces
├── iso_root/             # Files copied into the bootable ISO image
├── linker/               # Linker scripts
├── src/
│   ├── arch/i386/        # i386-specific implementation code
│   │   ├── cpu/
│   │   ├── hardware/
│   │   ├── interrupts/
│   │   └── memory/
│   ├── drivers/          # Device drivers
│   └── kernel/           # Generic kernel entry and tests
└── build/                # Generated objects, kernel binary, and ISO
```

## Prerequisites

Building and running wigOSX requires:
```sh
i686-elf-gcc
i686-elf-as
grub-mkrescue
qemu-system-i386
make
```

The compiler and assembler must target `i686-elf`, not the host operating
system. Using the host Linux compiler is intentionally rejected by the kernel
source.

## Build

```sh
make
```

The build output is generated under `build/`, and the temporary kernel copy
used by GRUB is generated at `iso_root/boot/wigOSX.bin`.

## Run 
```sh
make run
```

This builds the bootable ISO and starts it in QEMU:

```sh
qemu-system-i386 -cdrom build/iso/wigOSX.iso -serial stdio
```

Serial logs are printed to the terminal running QEMU.

## Clean
```sh
make clean
```

This removes generated build outputs and the temporary kernel copy in the ISO
root.

## Manual Testing
After booting, the kernel shell should appear:
```sh
wigOSX>
```

Useful commands include:
```sh
help
version
ticks
about
scroll
```
The `scroll` command prints enough lines to verify VGA terminal scrolling.