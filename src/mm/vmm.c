#include <kernel/arch/i386/paging.h>
#include <kernel/core/log.h>
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
  klog_writestring("Virtual memory manager: ");

  if (vmm_is_enabled()) {
    klog_writestring("enabled\n");
  } else {
    klog_writestring("disabled\n");
  }

  klog_writestring("Identity-mapped bytes: ");
  klog_write_uint32(vmm_get_identity_mapped_bytes());
  klog_putchar('\n');
}