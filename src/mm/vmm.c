#include <kernel/arch/i386/paging.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <kernel/mm/vmm.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file vmm.c
 * @brief Stage 14 generic virtual memory manager.
 *
 * This layer wraps the i386 paging implementation so higher kernel layers do
 * not need to know about CR0, CR3, page directories, or page tables directly.
 */

static bool vmm_initialized = false;

static void vmm_putchar(char c) {
  terminal_putchar(c);

  if (serial_is_initialized()) {
    serial_putchar(c);
  }
}

static void vmm_writestring(const char* text) {
  terminal_writestring(text);

  if (serial_is_initialized()) {
    serial_writestring(text);
  }
}

static void vmm_print_uint32(uint32_t value) {
  char digits[10];
  size_t digit_count = 0;

  if (value == 0) {
    vmm_putchar('0');
    return;
  }

  while (value > 0) {
    digits[digit_count] = '0' + (value % 10);
    value = value / 10;
    digit_count++;
  }

  while (digit_count > 0) {
    digit_count--;
    vmm_putchar(digits[digit_count]);
  }
}

void vmm_initialize(void) {
  i386_paging_initialize_identity();
  vmm_initialized = i386_paging_is_enabled();
}

bool vmm_is_enabled(void) {
  return vmm_initialized && i386_paging_is_enabled();
}

uint32_t vmm_get_identity_mapped_bytes(void) {
  return i386_paging_get_identity_mapped_bytes();
}

void vmm_print_summary(void) {
  vmm_writestring("Virtual memory manager: ");

  if (vmm_is_enabled()) {
    vmm_writestring("enabled\n");
  } else {
    vmm_writestring("disabled\n");
  }

  vmm_writestring("Identity-mapped bytes: ");
  vmm_print_uint32(vmm_get_identity_mapped_bytes());
  vmm_putchar('\n');
}