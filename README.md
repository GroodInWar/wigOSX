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
├── boot/                 # Multiboot entry assembly and GRUB config
├── include/kernel/       # Public kernel headers
│   ├── arch/i386/        # i386 CPU, GDT, IDT, I/O, and PIC headers
│   ├── boot/             # Boot protocol structures
│   ├── core/             # Generic kernel interfaces
│   ├── drivers/          # Driver interfaces
│   └── mm/               # Memory-management interfaces
├── linker/               # Linker scripts
├── src/
│   ├── arch/i386/        # i386-specific implementation code
│   │   ├── cpu/
│   │   ├── hardware/
│   │   ├── interrupts/
│   │   └── memory/
│   ├── drivers/          # Device drivers
│   ├── kernel/           # Generic kernel entry and tests
│   └── mm/               # Generic memory-management implementation
└── build/                # Generated objects, ISO staging, kernel binary, and ISO
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

The build output is generated under `build/`. The temporary ISO filesystem is
staged under `build/iso_root/`, including `build/iso_root/boot/wigOSX.bin` and
a copied `build/iso_root/boot/grub/grub.cfg`.

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

This removes generated build outputs, including the temporary ISO staging root.

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
mem
pmm
pmmtest
about
scroll
```
`version`  -> wigOSX 0.013.1
`about`    -> Stage 13.1: PMM cleanup and safety validation.
`mem`      -> Prints usable/reserved regions and total usable/reserved memory.
`pmm`      -> Prints physical frame counts.
`pmmtest`  -> Runs a basic allocate/free PMM test.
