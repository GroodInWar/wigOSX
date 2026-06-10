## @file Makefile
## @brief Builds the wigOSX kernel binary and bootable ISO image.

## @brief C cross-compiler used for freestanding i386 code.
CC = i686-elf-gcc

## @brief Assembler used for i386 assembly sources.
AS = i686-elf-as

## @brief Directory for generated object files and final images.
BUILD_DIR = build

## @brief Directory containing the bootable ISO filesystem layout.
ISO_DIR = iso

## @brief Linked kernel binary output path.
KERNEL_BIN = $(BUILD_DIR)/wigOSX.bin

## @brief Bootable ISO output path.
ISO_BIN = $(BUILD_DIR)/wigOSX.iso

## @brief Compiler flags for freestanding kernel C sources.
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude

## @brief Linker flags for the freestanding kernel image.
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

## @brief Object files linked into the kernel binary.
OBJS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/vga.o \
	$(BUILD_DIR)/serial.o \
	$(BUILD_DIR)/gdt.o \
	$(BUILD_DIR)/gdt_flush.o \
	$(BUILD_DIR)/test.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/idt_flush.o \
	$(BUILD_DIR)/isr_stubs.o

## @brief Default target that builds the bootable ISO image.
all: $(ISO_BIN)

## @brief Creates the build output directory.
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

## @brief Assembles the Multiboot entry point.
$(BUILD_DIR)/boot.o: boot/boot.s | $(BUILD_DIR)
	$(AS) boot/boot.s -o $(BUILD_DIR)/boot.o

## @brief Compiles the main kernel entry point.
$(BUILD_DIR)/kernel.o: kernel/kernel.c include/kernel/vga.h include/kernel/serial.h include/kernel/gdt.h include/kernel/idt.h include/kernel/test.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/kernel.c -o $(BUILD_DIR)/kernel.o

## @brief Compiles the VGA text-mode terminal driver.
$(BUILD_DIR)/vga.o: drivers/vga.c include/kernel/vga.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c drivers/vga.c -o $(BUILD_DIR)/vga.o

## @brief Compiles the COM1 serial logging driver.
$(BUILD_DIR)/serial.o: drivers/serial.c include/kernel/serial.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c drivers/serial.c -o $(BUILD_DIR)/serial.o

## @brief Compiles the i386 GDT setup code.
$(BUILD_DIR)/gdt.o: arch/i386/gdt.c include/kernel/gdt.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c arch/i386/gdt.c -o $(BUILD_DIR)/gdt.o

## @brief Assembles the i386 GDT loading helper.
$(BUILD_DIR)/gdt_flush.o: arch/i386/gdt_flush.s | $(BUILD_DIR)
	$(AS) arch/i386/gdt_flush.s -o $(BUILD_DIR)/gdt_flush.o

## @brief Compiles the i386 IDT setup code.
$(BUILD_DIR)/idt.o: arch/i386/idt.c include/kernel/idt.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c arch/i386/idt.c -o $(BUILD_DIR)/idt.o

## @brief Assembles the i386 IDT loading helper.
$(BUILD_DIR)/idt_flush.o: arch/i386/idt_flush.s | $(BUILD_DIR)
	$(AS) arch/i386/idt_flush.s -o $(BUILD_DIR)/idt_flush.o

## @brief Assembles the i386 CPU exception stubs.
$(BUILD_DIR)/isr_stubs.o: arch/i386/isr_stubs.s | $(BUILD_DIR)
	$(AS) arch/i386/isr_stubs.s -o $(BUILD_DIR)/isr_stubs.o

## @brief Compiles the kernel visual tests.
$(BUILD_DIR)/test.o: kernel/test.c include/kernel/test.h include/kernel/vga.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/test.c -o $(BUILD_DIR)/test.o

## @brief Links all kernel objects into the freestanding binary.
$(KERNEL_BIN): $(OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS) -lgcc

## @brief Copies the kernel into the ISO tree and creates a bootable ISO.
$(ISO_BIN): $(KERNEL_BIN) iso/boot/grub/grub.cfg
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/wigOSX.bin
	grub-mkrescue -o $(ISO_BIN) $(ISO_DIR)

## @brief Runs the kernel ISO in QEMU with serial output on stdio.
run: $(ISO_BIN)
	qemu-system-i386 -cdrom $(ISO_BIN) -serial stdio

## @brief Removes generated build outputs.
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ISO_DIR)/boot/wigOSX.bin
