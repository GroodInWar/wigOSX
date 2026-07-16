#ifndef KERNEL_MM_VMM_H
#define KERNEL_MM_VMM_H

#include <kernel/core/status.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @file vmm.h
 * @brief Generic virtual memory manager interface.
 *
 * Stage 14 only enables simple identity paging. Later stages can expand this
 * into real virtual address management.
 */

/**
 * @brief Initializes virtual memory and enables identity paging.
 *
 * @return KERNEL_STATUS_OK when paging is enabled, otherwise a hardware
 * failure status.
 */
kernel_status_t vmm_initialize(void);

/**
 * @brief Reports whether virtual memory is active.
 *
 * @return true if the VMM initialized and paging is currently enabled.
 */
bool vmm_is_enabled(void);

/**
 * @brief Returns the size of the current identity-mapped region.
 *
 * @return Number of identity-mapped bytes, or 0 when paging is not initialized.
 */
uint32_t vmm_get_identity_mapped_bytes(void);

/**
 * @brief Prints the current virtual memory summary.
 */
void vmm_print_summary(void);

#endif /* KERNEL_MM_VMM_H */
