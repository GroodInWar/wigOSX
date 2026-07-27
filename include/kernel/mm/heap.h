#ifndef KERNEL_MM_HEAP_H
#define KERNEL_MM_HEAP_H

#include <kernel/core/status.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file heap.h
 * @brief Generic kernel dynamic-memory allocation interface.
 *
 * The heap provides variable-sized allocations to kernel subsystems.
 *
 * This interface intentionally does not expose allocator metadata, physical
 * frames, page tables, or virtual-memory implementation details.
 */

/**
 * @brief Initializes the kernel heap backing region.
 *
 * Heap initialization must complete before kmalloc() or kfree() are used.
 *
 * @return KERNEL_STATUS_OK when the heap backing region is valid, otherwise an
 * appropriate failure status.
 */
kernel_status_t heap_initialize(void);

/**
 * @brief Returns whether the kernel heap has been initialized.
 */
bool heap_is_initialized(void);

/**
 * @brief Returns the first address owned by the initial kernel heap.
 */
uintptr_t heap_get_start_address(void);

/**
 * @brief Returns the first address after the initial kernel heap.
 */
uintptr_t heap_get_end_address(void);

/**
 * @brief Returns the total number of bytes in the initial heap region.
 */
size_t heap_get_capacity(void);

/**
 * @brief Allocates a variable-sized block of kernel memory.
 *
 * Stage 16.1 does not yet implement allocation.
 *
 * @param size Number of bytes requested.
 *
 * @return Pointer to usable kernel memory, or NULL if size is zero or the
 * allocation cannot be satisfied.
 */
void* kmalloc(size_t size);

/**
 * @brief Releases memory previously returned by kmalloc().
 *
 * Passing NULL performs no operation.
 *
 * @param pointer Previously allocated kernel-memory pointer, or NULL.
 */
void kfree(void* pointer);

#endif /* KERNEL_MM_HEAP_H */