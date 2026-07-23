#ifndef KERNEL_MM_HEAP_H
#define KERNEL_MM_HEAP_H

#include <kernel/core/status.h>
#include <stddef.h>

/**
 * @file heap.h
 * @brief Generic kernel dynamic-memory allocation interface.
 *
 * The heap provides variable-sized allocations to kernel subsystems.
 *
 * This interface intentioanlly does not expose allocator metadata, physical
 * frames, page tables, or virtual-memory implementation details.
 */

/**
 * @brief Intializes the kernel heap.
 *
 * Heap initialization must complete before kmalloc or kfree() are used.
 *
 * @return KERNEL_STATUS_OK when the heap is ready, otherwise an appropriate
 * failure status.
 */
kernel_status_t heap_initialize(void);

/**
 * @brief Allocates a variable-sized block of kernel memory.
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
 * Invalid allocations and allocator-corruption conditions may cause a kernel
 * panic rather than being silently ignored.
 *
 * @param pointer Previously allocated kernel-memory pointer, or NULL.
 */
void kfree(void* pointer);

#endif  // KERNEL_MM_HEAP_H
