#include <kernel/boot/multiboot.h>
#include <kernel/core/memory.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file memory.c
 * @brief Stage 11 Multiboot memory detection.
 *
 * This module only summarizes memory reported by the bootloader. It does not
 * choose free frames, enable paging, or provide allocation.
 */

static bool memory_basic_available = false;
static bool memory_map_available = false;

static uint32_t memory_lower_kb = 0;
static uint32_t memory_upper_kb = 0;

static uint32_t memory_usable_region_count = 0;
static uint32_t memory_reserved_region_count = 0;

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
 * @brief Prints an unsigned 32-bit integer in decimal.
 */
static void memory_print_uint32(uint32_t value) {
  char digits[10];
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
 * @brief Resets the stored memory summary.
 */
static void memory_reset_summary(void) {
  memory_basic_available = false;
  memory_map_available = false;

  memory_lower_kb = 0;
  memory_upper_kb = 0;

  memory_usable_region_count = 0;
  memory_reserved_region_count = 0;
}

/**
 * @brief Counts usable and reserved regions in the Multiboot memory map.
 */
static void memory_read_multiboot_map(const multiboot_info_t* info) {
  uint32_t current_address = info->mmap_addr;
  uint32_t end_address = info->mmap_addr + info->mmap_length;

  while (current_address < end_address) {
    const multiboot_memory_map_entry_t* entry =
        (const multiboot_memory_map_entry_t*)current_address;

    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
      memory_usable_region_count++;
    } else {
      memory_reserved_region_count++;
    }

    current_address += sizeof(entry->size) + entry->size;
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
  memory_writestring("Stage 11: Multiboot memory detection.\n");

  if (memory_basic_available) {
    memory_writestring("Lower memory: ");
    memory_print_uint32(memory_lower_kb);
    memory_writestring(" KB\n");

    memory_writestring("Upper memory: ");
    memory_print_uint32(memory_upper_kb);
    memory_writestring(" KB\n");
  } else {
    memory_writestring("Basic memory information: unavailable\n");
  }

  if (memory_map_available) {
    memory_writestring("Memory map: available\n");

    memory_writestring("Usable memory regions: ");
    memory_print_uint32(memory_usable_region_count);
    memory_putchar('\n');

    memory_writestring("Reserved memory regions: ");
    memory_print_uint32(memory_reserved_region_count);
    memory_putchar('\n');
  } else {
    memory_writestring("Memory map: unavailable\n");
  }
}