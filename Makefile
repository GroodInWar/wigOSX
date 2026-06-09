# Cross-compiler tools
CC = i686-elf-gcc
AS = i686-elf-as

# Project folders
BUILD_DIR = build
ISO_DIR = iso

# Output files
KERNEL_BIN = $(BUILD_DIR)/wigOSX.bin
ISO_BIN = $(BUILD_DIR)/wigOSX.iso

# Compiler and linker flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

# Object files used to build the kernel
OBJS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/vga.o \
	$(BUILD_DIR)/test.o

# Default target
all: $(ISO_BIN)

# Create build folder
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assemble boot code
$(BUILD_DIR)/boot.o: boot/boot.s | $(BUILD_DIR)
	$(AS) boot/boot.s -o $(BUILD_DIR)/boot.o

# Compile kernel
$(BUILD_DIR)/kernel.o: kernel/kernel.c include/kernel/vga.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/kernel.c -o $(BUILD_DIR)/kernel.o

# Compile VGA driver
$(BUILD_DIR)/vga.o: drivers/vga.c include/kernel/vga.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c drivers/vga.c -o $(BUILD_DIR)/vga.o

# Compile kernel tests
$(BUILD_DIR)/test.o: kernel/test.c include/kernel/test.h include/kernel/vga.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/test.c -o $(BUILD_DIR)/test.o

# Link kernel binary
$(KERNEL_BIN): $(OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS) -lgcc

# Build bootable ISO
$(ISO_BIN): $(KERNEL_BIN) iso/boot/grub/grub.cfg
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/wigOSX.bin
	grub-mkrescue -o $(ISO_BIN) $(ISO_DIR)

# Run in QEMU
run: $(ISO_BIN)
	qemu-system-i386 -cdrom $(ISO_BIN)

# Clean generated files
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ISO_DIR)/boot/wigOSX.bin