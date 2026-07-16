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

/**
 * @brief Size of one i386 page in bytes.
 */
#define I386_PAGING_PAGE_SIZE 4096U

/**
 * @brief Number of low physical bytes identity-mapped during early boot.
 */
#define I386_PAGING_IDENTITY_MAPPED_BYTES (16U * 1024U * 1024U)

/**
 * @brief Builds static identity page tables and enables CPU paging.
 */
void i386_paging_initialize_identity(void);

/**
 * @brief Reports whether the CPU paging bit is currently enabled.
 *
 * @return true when CR0.PG is set.
 */
bool i386_paging_is_enabled(void);

/**
 * @brief Returns the number of bytes identity-mapped by this paging layer.
 *
 * @return Identity-map size when initialized, otherwise 0.
 */
uint32_t i386_paging_get_identity_mapped_bytes(void);

#endif /* KERNEL_ARCH_I386_PAGING_H */
