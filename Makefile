## @file Makefile
## @brief Builds the wigOSX kernel binary and bootable ISO image.

## @brief C cross-compiler used for freestanding i386 code.
CC = i686-elf-gcc

## @brief Assembler used for i386 assembly sources.
AS = i686-elf-as

## @brief Root directory for kernel source files.
SRC_DIR = src

## @brief Directory containing boot inputs and the Multiboot entry assembly.
BOOT_DIR = boot

## @brief Directory containing public kernel headers.
INCLUDE_DIR = include

## @brief Directory containing linker scripts.
LINKER_DIR = linker

## @brief Directory for generated object files and final images.
BUILD_DIR = build

## @brief Directory for the generated bootable ISO filesystem staging area.
ISO_ROOT = $(BUILD_DIR)/iso_root

## @brief Directory for generated object files.
OBJ_DIR = $(BUILD_DIR)/objects

## @brief Directory for the linked kernel binary.
KERNEL_BUILD_DIR = $(BUILD_DIR)/kernel

## @brief Directory for the bootable ISO output.
ISO_BUILD_DIR = $(BUILD_DIR)/iso

## @brief Linker script used for the freestanding kernel image.
LINKER_SCRIPT = $(LINKER_DIR)/kernel.ld

## @brief Linked kernel binary output path.
KERNEL_BIN = $(KERNEL_BUILD_DIR)/wigOSX.bin

## @brief Bootable ISO output path.
ISO_BIN = $(ISO_BUILD_DIR)/wigOSX.iso

## @brief GRUB menu configuration copied into the generated ISO staging area.
GRUB_CFG = $(BOOT_DIR)/grub/grub.cfg

## @brief Compiler flags for freestanding kernel C sources.
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(INCLUDE_DIR)

## @brief Dependency-generation flags for C source files.
DEPFLAGS = -MMD -MP

## @brief Linker flags for the freestanding kernel image.
LDFLAGS = -T $(LINKER_SCRIPT) -ffreestanding -O2 -nostdlib

## @brief Object files linked into the kernel binary.
OBJS = \
	$(OBJ_DIR)/boot/multiboot.o \
	$(OBJ_DIR)/kernel/kernel.o \
	$(OBJ_DIR)/kernel/memory.o \
	$(OBJ_DIR)/mm/pmm.o \
	$(OBJ_DIR)/mm/vmm.o \
	$(OBJ_DIR)/kernel/shell.o \
	$(OBJ_DIR)/drivers/vga.o \
	$(OBJ_DIR)/drivers/serial.o \
	$(OBJ_DIR)/arch/i386/cpu/cpu.o \
	$(OBJ_DIR)/arch/i386/cpu/io.o \
	$(OBJ_DIR)/arch/i386/hardware/pic.o \
	$(OBJ_DIR)/drivers/pit.o \
	$(OBJ_DIR)/drivers/keyboard.o \
	$(OBJ_DIR)/arch/i386/memory/gdt.o \
	$(OBJ_DIR)/arch/i386/memory/gdt_flush.o \
	$(OBJ_DIR)/arch/i386/memory/paging.o \
	$(OBJ_DIR)/arch/i386/interrupts/idt.o \
	$(OBJ_DIR)/arch/i386/interrupts/idt_flush.o \
	$(OBJ_DIR)/arch/i386/interrupts/isr_stubs.o \
	$(OBJ_DIR)/kernel/test.o

## @brief Generated dependency files emitted by the C compiler.
DEPS = $(OBJS:.o=.d)

## @brief Default target that builds the bootable ISO image.
all: $(ISO_BIN)

## @brief Compiles generic kernel C sources.
$(OBJ_DIR)/kernel/%.o: $(SRC_DIR)/kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## @brief Compiles generic memory-management C sources.
$(OBJ_DIR)/mm/%.o: $(SRC_DIR)/mm/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## @brief Compiles generic device driver C sources.
$(OBJ_DIR)/drivers/%.o: $(SRC_DIR)/drivers/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## @brief Compiles i386 CPU helper C sources.
$(OBJ_DIR)/arch/i386/cpu/%.o: $(SRC_DIR)/arch/i386/cpu/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## @brief Compiles i386 hardware-controller C sources.
$(OBJ_DIR)/arch/i386/hardware/%.o: $(SRC_DIR)/arch/i386/hardware/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## @brief Compiles i386 memory-management C sources.
$(OBJ_DIR)/arch/i386/memory/%.o: $(SRC_DIR)/arch/i386/memory/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## @brief Compiles i386 interrupt C sources.
$(OBJ_DIR)/arch/i386/interrupts/%.o: $(SRC_DIR)/arch/i386/interrupts/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## @brief Assembles the Multiboot entry point.
$(OBJ_DIR)/boot/%.o: $(BOOT_DIR)/%.s
	mkdir -p $(dir $@)
	$(AS) $< -o $@

## @brief Assembles i386 memory-management helpers.
$(OBJ_DIR)/arch/i386/memory/%.o: $(SRC_DIR)/arch/i386/memory/%.s
	mkdir -p $(dir $@)
	$(AS) $< -o $@

## @brief Assembles i386 interrupt helpers.
$(OBJ_DIR)/arch/i386/interrupts/%.o: $(SRC_DIR)/arch/i386/interrupts/%.s
	mkdir -p $(dir $@)
	$(AS) $< -o $@

## @brief Links all kernel objects into the freestanding binary.
$(KERNEL_BIN): $(OBJS) $(LINKER_SCRIPT)
	mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS) -lgcc

## @brief Stages GRUB inputs and the kernel binary, then creates a bootable ISO.
$(ISO_BIN): $(KERNEL_BIN) $(GRUB_CFG)
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)/boot/grub $(dir $@)
	cp $(GRUB_CFG) $(ISO_ROOT)/boot/grub/grub.cfg
	cp $(KERNEL_BIN) $(ISO_ROOT)/boot/wigOSX.bin
	grub-mkrescue -o $(ISO_BIN) $(ISO_ROOT)

## @brief Runs the kernel ISO in QEMU with serial output on stdio.
run: $(ISO_BIN)
	qemu-system-i386 -cdrom $(ISO_BIN) -serial stdio

## @brief Removes generated build outputs.
clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
