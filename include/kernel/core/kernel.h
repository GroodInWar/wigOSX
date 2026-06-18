#ifndef KERNEL_CORE_KERNEL_H
#define KERNEL_CORE_KERNEL_H

#include <stdint.h>

/**
 * @file kernel.h
 * @brief Public declaration for the kernel's C entry point.
 */

/**
 * @brief Main kernel entry point called by the Multiboot assembly bootstrap.
 *
 * @param multiboot_magic Magic value passed by the Multiboot bootloader.
 * @param multiboot_info_address Physical address of the Multiboot information
 * structure.
 *
 * This function initializes the kernel subsystems and then enters the idle
 * loop.
 */
void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info_address);

#endif /* KERNEL_CORE_KERNEL_H */
