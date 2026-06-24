#include <kernel/boot/multiboot.h>
#include <kernel/core/memory.h>
#include <kernel/core/version.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
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
 * @brief Writes one character to both VGA and serial output.
 */
static void memory_putchar(char c) {
  terminal_putchar(c);

  if (serial_is_initialized()) {
    serial_putchar(c);
  }
}

/**
 * @brief Writes a string to both VGA and serial output.
 */
static void memory_writestring(const char* text) {
  terminal_writestring(text);

  if (serial_is_initialized()) {
    serial_writestring(text);
  }
}

/**
 * @brief Prints an unsigned 64-bit integer in decimal.
 */
static void memory_print_uint64(uint64_t value) {
  char digits[20];
  size_t digit_count = 0;

  if (value == 0) {
    memory_putchar('0');
    return;
  }

  while (value > 0) {
    digits[digit_count] = '0' + (value % 10);
    value = value / 10;
    digit_count++;
  }

  while (digit_count > 0) {
    digit_count--;
    memory_putchar(digits[digit_count]);
  }
}

/**
 * @brief Prints a byte value as whole KiB.
 */
static void memory_print_kib(uint64_t bytes) {
  memory_print_uint64(bytes / 1024);
  memory_writestring(" KB");
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
static void memory_read_multiboot_map(const multiboot_info_t* info) {
  uint32_t current_address = info->mmap_addr;
  uint32_t end_address = info->mmap_addr + info->mmap_length;

  if (end_address < current_address) {
    return;
  }

  while ((end_address - current_address) >= sizeof(uint32_t)) {
    const multiboot_memory_map_entry_t* entry =
        (const multiboot_memory_map_entry_t*)current_address;

    if (entry->size < MULTIBOOT_MEMORY_MAP_BASE_SIZE) {
      break;
    }

    uint32_t entry_total_size = sizeof(entry->size) + entry->size;

    if (entry_total_size > (end_address - current_address)) {
      break;
    }

    memory_region_type_t region_type = MEMORY_REGION_RESERVED;

    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
      region_type = MEMORY_REGION_USABLE;
    }

    memory_add_region(entry->base_addr, entry->length, region_type);

    current_address += entry_total_size;
  }
}

void memory_initialize(uint32_t multiboot_info_address) {
  const multiboot_info_t* info =
      (const multiboot_info_t*)multiboot_info_address;

  memory_reset_summary();

  if ((info->flags & MULTIBOOT_INFO_MEMORY) != 0) {
    memory_basic_available = true;
    memory_lower_kb = info->mem_lower;
    memory_upper_kb = info->mem_upper;
  }

  if ((info->flags & MULTIBOOT_INFO_MEMORY_MAP) != 0 && info->mmap_addr != 0 &&
      info->mmap_length > 0) {
    memory_map_available = true;
    memory_read_multiboot_map(info);
  }
}

void memory_print_summary(void) {
  memory_writestring(WIGOSX_STAGE_LABEL);
  memory_writestring(".\n");

  if (memory_basic_available) {
    memory_writestring("Lower memory: ");
    memory_print_uint64(memory_lower_kb);
    memory_writestring(" KB\n");

    memory_writestring("Upper memory: ");
    memory_print_uint64(memory_upper_kb);
    memory_writestring(" KB\n");
  } else {
    memory_writestring("Basic memory information: unavailable\n");
  }

  if (!memory_map_available) {
    memory_writestring("Memory map: unavailable\n");
    return;
  }

  memory_writestring("Memory map: available and normalized\n");

  memory_writestring("Stored memory regions: ");
  memory_print_uint64(memory_region_count);
  memory_putchar('\n');

  memory_writestring("Usable memory regions: ");
  memory_print_uint64(memory_usable_region_count);
  memory_putchar('\n');

  memory_writestring("Reserved memory regions: ");
  memory_print_uint64(memory_reserved_region_count);
  memory_putchar('\n');

  memory_writestring("Total usable memory: ");
  memory_print_kib(memory_total_usable_bytes);
  memory_putchar('\n');

  memory_writestring("Total reserved memory: ");
  memory_print_kib(memory_total_reserved_bytes);
  memory_putchar('\n');

  if (memory_map_truncated) {
    memory_writestring(
        "WARNING: memory map truncated; increase MEMORY_MAX_REGIONS.\n");
  }

  for (uint32_t i = 0; i < memory_region_count; i++) {
    memory_writestring("Region ");
    memory_print_uint64(i);
    memory_writestring(": ");

    memory_writestring(memory_region_type_name(memory_regions[i].type));

    memory_writestring(", base=");
    memory_print_kib(memory_regions[i].base_address);

    memory_writestring(", length=");
    memory_print_kib(memory_regions[i].length);

    memory_putchar('\n');
  }
}

uint32_t memory_get_region_count(void) { return memory_region_count; }

const memory_region_t* memory_get_region(uint32_t index) {
  if (index >= memory_region_count) {
    return NULL;
  }

  return &memory_regions[index];
}

uint64_t memory_get_total_usable_bytes(void) {
  return memory_total_usable_bytes;
}

uint64_t memory_get_total_reserved_bytes(void) {
  return memory_total_reserved_bytes;
}

bool memory_was_map_truncated(void) { return memory_map_truncated; }