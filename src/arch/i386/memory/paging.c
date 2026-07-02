#include <kernel/arch/i386/paging.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file paging.c
 * @brief Stage 14 i386 identity paging setup.
 *
 * Stage 14 uses static page tables and identity maps the first 16 MiB:
 *
 * virtual address == physical address
 *
 * This keeps the kernel simple while introducing paging safely.
 */

#define I386_PAGE_DIRECTORY_ENTRY_COUNT 1024U
#define I386_PAGE_TABLE_ENTRY_COUNT 1024U

#define I386_PAGE_PRESENT 0x001U
#define I386_PAGE_WRITABLE 0x002U
#define I386_PAGE_FLAGS (I386_PAGE_PRESENT | I386_PAGE_WRITABLE)

#define I386_CR0_PAGING_ENABLE 0x80000000U

#define I386_PAGE_TABLE_COUNT          \
  (I386_PAGING_IDENTITY_MAPPED_BYTES / \
   (I386_PAGING_PAGE_SIZE * I386_PAGE_TABLE_ENTRY_COUNT))

static uint32_t paging_directory[I386_PAGE_DIRECTORY_ENTRY_COUNT]
    __attribute__((aligned(I386_PAGING_PAGE_SIZE)));

static uint32_t paging_tables[I386_PAGE_TABLE_COUNT]
                             [I386_PAGE_TABLE_ENTRY_COUNT]
    __attribute__((aligned(I386_PAGING_PAGE_SIZE)));

static bool paging_initialized = false;

static uint32_t i386_read_cr0(void) {
  uint32_t value = 0;
  __asm__ volatile("mov %%cr0, %0" : "=r"(value));
  return value;
}

static void i386_write_cr0(uint32_t value) {
  __asm__ volatile("mov %0, %%cr0" : : "r"(value) : "memory");
}

static void i386_write_cr3(uint32_t value) {
  __asm__ volatile("mov %0, %%cr3" : : "r"(value) : "memory");
}

static void i386_paging_clear_tables(void) {
  for (uint32_t i = 0; i < I386_PAGE_DIRECTORY_ENTRY_COUNT; i++) {
    paging_directory[i] = 0;
  }

  for (uint32_t table = 0; table < I386_PAGE_TABLE_COUNT; table++) {
    for (uint32_t page = 0; page < I386_PAGE_TABLE_ENTRY_COUNT; page++) {
      paging_tables[table][page] = 0;
    }
  }
}

static void i386_paging_build_identity_map(void) {
  for (uint32_t table = 0; table < I386_PAGE_TABLE_COUNT; table++) {
    for (uint32_t page = 0; page < I386_PAGE_TABLE_ENTRY_COUNT; page++) {
      uint32_t page_index = table * I386_PAGE_TABLE_ENTRY_COUNT + page;
      uint32_t physical_address = page_index * I386_PAGING_PAGE_SIZE;

      paging_tables[table][page] = physical_address | I386_PAGE_FLAGS;
    }

    paging_directory[table] =
        ((uint32_t)(uintptr_t)paging_tables[table]) | I386_PAGE_FLAGS;
  }
}

void i386_paging_initialize_identity(void) {
  i386_paging_clear_tables();
  i386_paging_build_identity_map();

  i386_write_cr3((uint32_t)(uintptr_t)paging_directory);
  i386_write_cr0(i386_read_cr0() | I386_CR0_PAGING_ENABLE);

  paging_initialized = i386_paging_is_enabled();
}

bool i386_paging_is_enabled(void) {
  return (i386_read_cr0() & I386_CR0_PAGING_ENABLE) != 0;
}

uint32_t i386_paging_get_identity_mapped_bytes(void) {
  if (!paging_initialized) {
    return 0;
  }

  return I386_PAGING_IDENTITY_MAPPED_BYTES;
}