#include <kernel/mm/heap.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file heap.c
 * @brief Initial kernel heap backing-region ownership.
 *
 * Stage 16.1 validates and exposes a fixed contiguous heap region reserved by
 * the linker. Allocation and free-list metadata are introduced later.
 */

/**
 * @brief Linker-provided first byte of the reserved heap region.
 */
extern uint8_t _kernel_heap_start;

/**
 * @brief Linker-provided first byte after the reserved heap region.
 */
extern uint8_t _kernel_heap_end;

/**
 * @brief Whether the heap backing region passed initialization checks.
 */
static bool heap_initialized = false;

/**
 * @brief Cached inclusive starting address of the heap region.
 */
static uintptr_t heap_start_address = 0;

/**
 * @brief Cached exclusive ending address of the heap region.
 */
static uintptr_t heap_end_address = 0;

/**
 * @brief Cached size of the heap region in bytes.
 */
static size_t heap_capacity = 0;

/**
 * @brief Initializes and validates the linker-reserved heap region.
 */
kernel_status_t heap_initialize(void) {
  uintptr_t start = (uintptr_t)&_kernel_heap_start;
  uintptr_t end = (uintptr_t)&_kernel_heap_end;

  if (heap_initialized) {
    return KERNEL_STATUS_ALREADY_INITIALIZED;
  }

  if (start == 0 || end == 0 || end <= start) {
    return KERNEL_STATUS_INVALID_ARGUMENT;
  }

  /*
   * The backing region is page-aligned so later stages can safely integrate it
   * with page-level virtual and physical memory management.
   */
  if ((start % 4096U) != 0 || (end % 4096U) != 0) {
    return KERNEL_STATUS_INVALID_ARGUMENT;
  }

  heap_start_address = start;
  heap_end_address = end;
  heap_capacity = (size_t)(end - start);
  heap_initialized = true;

  return KERNEL_STATUS_OK;
}

bool heap_is_initialized(void) { return heap_initialized; }

uintptr_t heap_get_start_address(void) {
  if (!heap_initialized) {
    return 0;
  }

  return heap_start_address;
}

uintptr_t heap_get_end_address(void) {
  if (!heap_initialized) {
    return 0;
  }

  return heap_end_address;
}

size_t heap_get_capacity(void) {
  if (!heap_initialized) {
    return 0;
  }

  return heap_capacity;
}

void* kmalloc(size_t size) {
  (void)size;

  /*
   * Stage 16.2 introduces allocation metadata and first-fit allocation.
   */
  return NULL;
}

void kfree(void* pointer) {
  (void)pointer;

  /*
   * Stage 16.2 introduces allocated blocks that can be released.
   */
}