#include <kernel/mm/heap.h>
#include <stddef.h>

/**
 * @file heap.c
 * @brief Kernel dynamic-memory allocator implementation.
 *
 * Stage 16.0 defines the heap ownership and public interface only.
 * Backing-memory selection and allocation logic are introduced in later
 * Stage 16 steps.
 */

kernel_status_t heap_initialize(void) {
  /*
   * Stage 16.1 will establish the initial heap backing region.
   */
  return KERNEL_STATUS_UNAVAILABLE;
}

void* kmalloc(size_t size) {
  (void)size;

  /*
   * Allocation is intentionally unavailable until the heap has backing memory
   * and allocator metadata.
   */
  return NULL;
}

void kfree(void* pointer) {
  (void)pointer;

  /*
   * No allocations can exist during Stage 16.0.
   */
}