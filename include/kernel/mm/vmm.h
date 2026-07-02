#ifndef KERNEL_MM_VMM_H
#define KERNEL_MM_VMM_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file vmm.h
 * @brief Generic virtual memory manager interface.
 *
 * Stage 14 only enables simple identity paging. Later stages can expand this
 * into real virtual address management.
 */

void vmm_initialize(void);
bool vmm_is_enabled(void);
uint32_t vmm_get_identity_mapped_bytes(void);
void vmm_print_summary(void);

#endif /* KERNEL_MM_VMM_H */