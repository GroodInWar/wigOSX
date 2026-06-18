#ifndef KERNEL_CORE_KERNEL_H
#define KERNEL_CORE_KERNEL_H

/**
 * @file kernel.h
 * @brief Public declaration for the kernel's C entry point.
 */

/**
 * @brief Main kernel entry point called by the Multiboot assembly bootstrap.
 *
 * This function initializes the kernel subsystems and then enters the idle
 * loop.
 */
void kernel_main(void);

#endif /* KERNEL_CORE_KERNEL_H */
