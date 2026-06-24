#ifndef KERNEL_CORE_MEMORY_H
#define KERNEL_CORE_MEMORY_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file memory.h
 * @brief Stage 12 normalized kernel memory map interface.
 *
 * Stage 12 still does not allocate memory. It converts Multiboot memory
 * information into a small kernel-owned memory map that later memory managers
 * can use.
 */

#define MEMORY_MAX_REGIONS 32

typedef enum {
  MEMORY_REGION_USABLE = 1,
  MEMORY_REGION_RESERVED = 2
} memory_region_type_t;

typedef struct {
  uint64_t base_address;
  uint64_t length;
  memory_region_type_t type;
} memory_region_t;

/**
 * @brief Reads Multiboot memory information and builds a normalized map.
 *
 * @param multiboot_info_address Physical address of the Multiboot information
 * structure provided by the bootloader.
 */
void memory_initialize(uint32_t multiboot_info_address);

/**
 * @brief Prints the detected and normalized memory map.
 */
void memory_print_summary(void);

/**
 * @brief Returns the number of normalized memory regions stored by the kernel.
 */
uint32_t memory_get_region_count(void);

/**
 * @brief Returns one normalized memory region, or null if index is invalid.
 */
const memory_region_t* memory_get_region(uint32_t index);

/**
 * @brief Returns the total usable memory found in the Multiboot memory map.
 */
uint64_t memory_get_total_usable_bytes(void);

/**
 * @brief Returns the total reserved memory found in the Multiboot memory map.
 */
uint64_t memory_get_total_reserved_bytes(void);

/**
 * @brief Returns true if the Multiboot memory map had more regions than the
 * fixed Stage 12 storage can hold.
 */
bool memory_was_map_truncated(void);

#endif /* KERNEL_CORE_MEMORY_H */