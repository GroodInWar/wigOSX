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

## Build

```sh
make
```

The build output is generated under `build/`, and the temporary kernel copy
used by GRUB is generated at `iso_root/boot/wigOSX.bin`.
