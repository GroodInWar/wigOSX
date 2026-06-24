#ifndef KERNEL_BOOT_MULTIBOOT_H
#define KERNEL_BOOT_MULTIBOOT_H

#include <stdint.h>

/**
 * @file multiboot.h
 * @brief Minimal Multiboot information structures used by wigOSX.
 *
 * Stage 11 only reads memory-related boot information. The kernel should check
 * the matching flag before reading any optional Multiboot field.
 */

/** @brief mem_lower and mem_upper are valid. */
#define MULTIBOOT_INFO_MEMORY (1 << 0)

/** @brief boot_device is valid. */
#define MULTIBOOT_INFO_BOOT_DEVICE (1 << 1)

/** @brief cmdline is valid. */
#define MULTIBOOT_INFO_COMMAND_LINE (1 << 2)

/** @brief mods_count and mods_addr are valid. */
#define MULTIBOOT_INFO_MODULES (1 << 3)

/** @brief mmap_length and mmap_addr are valid. */
#define MULTIBOOT_INFO_MEMORY_MAP (1 << 6)

/** @brief Multiboot memory-map type for usable RAM. */
#define MULTIBOOT_MEMORY_AVAILABLE 1

/**
 * @brief Partial Multiboot information structure.
 *
 * This layout includes the early fields needed to reach the memory-map fields.
 * More fields can be added later when the kernel needs bootloader name, APM,
 * VBE, framebuffer, or ELF section information.
 */
typedef struct {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  uint32_t syms[4];
  uint32_t mmap_length;
  uint32_t mmap_addr;
} multiboot_info_t;

/**
 * @brief One variable-sized Multiboot memory-map entry.
 *
 * The size field is not included in the size value itself. To move to the next
 * entry, add sizeof(size) + entry->size.
 */
typedef struct {
  uint32_t size;
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
} __attribute__((packed)) multiboot_memory_map_entry_t;

#endif /* KERNEL_BOOT_MULTIBOOT_H */