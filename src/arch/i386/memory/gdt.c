#include <kernel/arch/i386/gdt.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file gdt.c
 * @brief Builds the i386 Global Descriptor Table.
 *
 * This file creates the GDT descriptors used by the kernel and calls
 * the assembly helper that loads the GDT into the CPU.
 */

/**
 * @brief One 8-byte GDT descriptor.
 *
 * The CPU expects this exact binary layout. The packed attribute is required
 * because compiler padding would break the descriptor format.
 */
struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

/**
 * @brief Pointer structure loaded by the lgdt instruction.
 *
 * The CPU's GDTR register receives a limit and a base address.
 */
struct gdt_pointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

/**
 * @brief Number of descriptors in the Stage 4 GDT.
 *
 * Stage 4 uses:
 * - null descriptor
 * - kernel code descriptor
 * - kernel data descriptor
 */
#define GDT_ENTRY_COUNT 3

static struct gdt_entry gdt_entries[GDT_ENTRY_COUNT];
static struct gdt_pointer gdt_ptr;

/**
 * @brief Assembly helper that loads the GDT and reloads segment registers.
 *
 * @param gdt_ptr_address Address of the GDT pointer structure.
 */
extern void gdt_flush(uint32_t gdt_ptr_address);

/**
 * @brief Writes one descriptor into the GDT.
 *
 * @param index GDT slot to write.
 * @param base Base address of the segment.
 * @param limit Segment limit.
 * @param access Access byte describing type, privilege, and present bit.
 * @param granularity Granularity and size flags.
 *
 * The GDT format splits the base address and limit across several fields.
 * This helper hides that layout from the rest of the kernel.
 */
static void gdt_set_entry(size_t index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t granularity) {
  gdt_entries[index].base_low = base & 0xFFFF;
  gdt_entries[index].base_middle = (base >> 16) & 0xFF;
  gdt_entries[index].base_high = (base >> 24) & 0xFF;

  gdt_entries[index].limit_low = limit & 0xFFFF;
  gdt_entries[index].granularity = (limit >> 16) & 0x0F;
  gdt_entries[index].granularity |= granularity & 0xF0;

  gdt_entries[index].access = access;
}

/**
 * @brief Builds and loads the kernel GDT.
 *
 * This creates a minimal flat-memory GDT:
 * - descriptor 0: required null descriptor
 * - descriptor 1: kernel code segment
 * - descriptor 2: kernel data segment
 *
 * The code and data descriptors both use base 0 and a 4 GiB effective
 * range. This does not provide memory isolation yet; it simply gives the
 * kernel explicit control over its protected-mode segment setup.
 */
void gdt_initialize(void) {
  gdt_ptr.limit = sizeof(gdt_entries) - 1;
  gdt_ptr.base = (uint32_t)&gdt_entries[0];

  gdt_set_entry(GDT_NULL_INDEX, 0, 0, 0, 0);

  gdt_set_entry(GDT_KERNEL_CODE_INDEX, GDT_FLAT_BASE, GDT_FLAT_LIMIT,
                GDT_ACCESS_KERNEL_CODE, GDT_GRANULARITY_4K_32BIT);

  gdt_set_entry(GDT_KERNEL_DATA_INDEX, GDT_FLAT_BASE, GDT_FLAT_LIMIT,
                GDT_ACCESS_KERNEL_DATA, GDT_GRANULARITY_4K_32BIT);

  gdt_flush((uint32_t)&gdt_ptr);
}