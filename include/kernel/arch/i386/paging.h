#ifndef KERNEL_ARCH_I386_PAGING_H
#define KERNEL_ARCH_I386_PAGING_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file paging.h
 * @brief i386 paging hardware interface.
 *
 * This layer owns i386-specific page-directory/page-table setup and CPU paging
 * control. Generic kernel code should prefer the higher-level VMM interface.
 */

#define I386_PAGING_PAGE_SIZE 4096U
#define I386_PAGING_IDENTITY_MAPPED_BYTES (16U * 1024U * 1024U)

void i386_paging_initialize_identity(void);
bool i386_paging_is_enabled(void);
uint32_t i386_paging_get_identity_mapped_bytes(void);

#endif /* KERNEL_ARCH_I386_PAGING_H */