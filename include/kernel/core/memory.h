#ifndef KERNEL_CORE_MEMORY_H
#define KERNEL_CORE_MEMORY_H

#include <stdint.h>

/**
 * @file memory.h
 * @brief Stage 11 memory detection interface.
 *
 * This does not allocate memory yet. It only reads the Multiboot memory
 * information provided by GRUB and stores a small summary for later stages.
 */

/**
 * @brief Reads Multiboot memory information and stores a kernel memory summary.
 *
 * @param multiboot_info_address Physical address of the Multiboot information
 * structure provided by the bootloader.
 */
void memory_initialize(uint32_t multiboot_info_address);

/**
 * @brief Prints the memory summary detected during memory_initialize().
 */
void memory_print_summary(void);

#endif /* KERNEL_CORE_MEMORY_H */