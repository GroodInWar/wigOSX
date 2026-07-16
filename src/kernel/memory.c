#include <kernel/boot/multiboot.h>
#include <kernel/core/log.h>
#include <kernel/core/memory.h>
#include <kernel/core/status.h>
#include <kernel/core/version.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file memory.c
 * @brief Stage 12 normalized kernel memory map.
 *
 * This module reads memory information provided by the Multiboot bootloader
 * and converts it into a small kernel-owned map. It still does not allocate
 * memory, enable paging, or manage free frames.
 */

#define MULTIBOOT_MEMORY_MAP_BASE_SIZE \
  (sizeof(multiboot_memory_map_entry_t) - sizeof(uint32_t))

static bool memory_initialized = false;
static bool memory_basic_available = false;
static bool memory_map_available = false;
static bool memory_map_truncated = false;

static uint32_t memory_lower_kb = 0;
static uint32_t memory_upper_kb = 0;

static uint32_t memory_usable_region_count = 0;
static uint32_t memory_reserved_region_count = 0;

static uint64_t memory_total_usable_bytes = 0;
static uint64_t memory_total_reserved_bytes = 0;

static memory_region_t memory_regions[MEMORY_MAX_REGIONS];
static uint32_t memory_region_count = 0;

/**
 * @brief Prints a byte value as whole KiB.
 */
static void memory_print_kib(uint64_t bytes) {
  klog_write_uint64(bytes / 1024ULL);
  klog_writestring(" KB");
}

/**
 * @brief Returns a readable name for a normalized region type.
 */
static const char* memory_region_type_name(memory_region_type_t type) {
  if (type == MEMORY_REGION_USABLE) {
    return "usable";
  }

  return "reserved";
}

/**
 * @brief Resets the stored memory summary and normalized map.
 */
static void memory_reset_summary(void) {
  memory_initialized = false;
  memory_basic_available = false;
  memory_map_available = false;
  memory_map_truncated = false;

  memory_lower_kb = 0;
  memory_upper_kb = 0;

  memory_usable_region_count = 0;
  memory_reserved_region_count = 0;

  memory_total_usable_bytes = 0;
  memory_total_reserved_bytes = 0;

  for (uint32_t i = 0; i < MEMORY_MAX_REGIONS; i++) {
    memory_regions[i].base_address = 0;
    memory_regions[i].length = 0;
    memory_regions[i].type = MEMORY_REGION_RESERVED;
  }

  memory_region_count = 0;
}

/**
 * @brief Adds one normalized memory region to the kernel-owned map.
 *
 * @param base_address Physical start address of the region.
 * @param length Region size in bytes.
 * @param type Normalized region type.
 */
static void memory_add_region(uint64_t base_address, uint64_t length,
                              memory_region_type_t type) {
  if (length == 0) {
    return;
  }

  if (type == MEMORY_REGION_USABLE) {
    memory_usable_region_count++;
    memory_total_usable_bytes += length;
  } else {
    memory_reserved_region_count++;
    memory_total_reserved_bytes += length;
  }

  if (memory_region_count >= MEMORY_MAX_REGIONS) {
    memory_map_truncated = true;
    return;
  }

  memory_regions[memory_region_count].base_address = base_address;
  memory_regions[memory_region_count].length = length;
  memory_regions[memory_region_count].type = type;
  memory_region_count++;
}

/**
 * @brief Converts the Multiboot memory map into normalized kernel regions.
 */
static kernel_status_t memory_read_multiboot_map(const multiboot_info_t* info) {
  if (info == NULL || info->mmap_addr == 0 || info->mmap_length == 0) {
    return KERNEL_STATUS_INVALID_ARGUMENT;
  }

  uint32_t current_address = info->mmap_addr;

  if (info->mmap_length > UINT32_MAX - current_address) {
    return KERNEL_STATUS_MALFORMED_DATA;
  }

  uint32_t end_address = current_address + info->mmap_length;

  while (current_address < end_address) {
    uint32_t remaining_bytes = end_address - current_address;

    if (remaining_bytes < sizeof(uint32_t)) {
      return KERNEL_STATUS_MALFORMED_DATA;
    }

    const multiboot_memory_map_entry_t* entry =
        (const multiboot_memory_map_entry_t*)current_address;

    if (entry->size < MULTIBOOT_MEMORY_MAP_BASE_SIZE) {
      return KERNEL_STATUS_MALFORMED_DATA;
    }

    if (entry->size > UINT32_MAX - (uint32_t)sizeof(entry->size)) {
      return KERNEL_STATUS_MALFORMED_DATA;
    }

    uint32_t entry_total_size = (uint32_t)sizeof(entry->size) + entry->size;

    if (entry_total_size > remaining_bytes) {
      return KERNEL_STATUS_MALFORMED_DATA;
    }

    if (entry->length != 0 && entry->base_addr > UINT64_MAX - entry->length) {
      return KERNEL_STATUS_MALFORMED_DATA;
    }

    memory_region_type_t region_type = MEMORY_REGION_RESERVED;

    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
      region_type = MEMORY_REGION_USABLE;
    }

    memory_add_region(entry->base_addr, entry->length, region_type);

    if (memory_map_truncated) {
      return KERNEL_STATUS_MALFORMED_DATA;
    }

    current_address += entry_total_size;
  }

  if (memory_region_count == 0) {
    return KERNEL_STATUS_MALFORMED_DATA;
  }

  return KERNEL_STATUS_OK;
}

/**
 * @brief Reads Multiboot memory information and builds a normalized map.
 */
kernel_status_t memory_initialize(uint32_t multiboot_info_address) {
  memory_reset_summary();

  if (multiboot_info_address == 0) {
    return KERNEL_STATUS_INVALID_ARGUMENT;
  }

  const multiboot_info_t* info =
      (const multiboot_info_t*)multiboot_info_address;

  if ((info->flags & MULTIBOOT_INFO_MEMORY) != 0) {
    memory_basic_available = true;
    memory_lower_kb = info->mem_lower;
    memory_upper_kb = info->mem_upper;
  }

  if ((info->flags & MULTIBOOT_INFO_MEMORY_MAP) == 0 || info->mmap_addr == 0 ||
      info->mmap_length == 0) {
    return KERNEL_STATUS_UNAVAILABLE;
  }

  kernel_status_t status = memory_read_multiboot_map(info);

  if (!kernel_status_is_ok(status)) {
    return status;
  }

  memory_map_available = true;
  memory_initialized = true;

  return KERNEL_STATUS_OK;
}

/**
 * @brief Reports whether the normalized memory map is ready.
 */
bool memory_is_initialized(void) { return memory_initialized; }

/**
 * @brief Prints the detected basic memory information and normalized map.
 */
void memory_print_summary(void) {
  klog_writestring(WIGOSX_STAGE_LABEL);
  klog_writestring(".\n");

  if (memory_basic_available) {
    klog_writestring("Lower memory: ");
    klog_write_uint64(memory_lower_kb);
    klog_writestring(" KB\n");

    klog_writestring("Upper memory: ");
    klog_write_uint64(memory_upper_kb);
    klog_writestring(" KB\n");
  } else {
    klog_writestring("Basic memory information: unavailable\n");
  }

  if (!memory_map_available) {
    klog_writestring("Memory map: unavailable\n");
    return;
  }

  klog_writestring("Memory map: available and normalized\n");

  klog_writestring("Stored memory regions: ");
  klog_write_uint64(memory_region_count);
  klog_putchar('\n');

  klog_writestring("Usable memory regions: ");
  klog_write_uint64(memory_usable_region_count);
  klog_putchar('\n');

  klog_writestring("Reserved memory regions: ");
  klog_write_uint64(memory_reserved_region_count);
  klog_putchar('\n');

  klog_writestring("Total usable memory: ");
  memory_print_kib(memory_total_usable_bytes);
  klog_putchar('\n');

  klog_writestring("Total reserved memory: ");
  memory_print_kib(memory_total_reserved_bytes);
  klog_putchar('\n');

  if (memory_map_truncated) {
    klog_writestring(
        "WARNING: memory map truncated; increase MEMORY_MAX_REGIONS.\n");
  }

  for (uint32_t i = 0; i < memory_region_count; i++) {
    klog_writestring("Region ");
    klog_write_uint64(i);
    klog_writestring(": ");

    klog_writestring(memory_region_type_name(memory_regions[i].type));

    klog_writestring(", base=");
    memory_print_kib(memory_regions[i].base_address);

    klog_writestring(", length=");
    memory_print_kib(memory_regions[i].length);

    klog_putchar('\n');
  }
}

/**
 * @brief Returns the number of stored normalized memory regions.
 */
uint32_t memory_get_region_count(void) { return memory_region_count; }

/**
 * @brief Returns one stored normalized memory region by index.
 */
const memory_region_t* memory_get_region(uint32_t index) {
  if (index >= memory_region_count) {
    return NULL;
  }

  return &memory_regions[index];
}

/**
 * @brief Returns total usable memory from the normalized map.
 */
uint64_t memory_get_total_usable_bytes(void) {
  return memory_total_usable_bytes;
}

/**
 * @brief Returns total reserved memory from the normalized map.
 */
uint64_t memory_get_total_reserved_bytes(void) {
  return memory_total_reserved_bytes;
}

/**
 * @brief Reports whether normalized memory-region storage overflowed.
 */
bool memory_was_map_truncated(void) { return memory_map_truncated; }
