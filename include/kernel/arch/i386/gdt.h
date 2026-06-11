#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <stdint.h>

/**
 * @file gdt.h
 * @brief Public interface and constants for the Global Descriptor Table.
 *
 * The GDT defines the CPU segment descriptors used by the kernel.
 * In Stage 4, wigOSX uses a simple flat memory model with one kernel
 * code segment and one kernel data segment.
 */

/**
 * @brief Size in bytes of one GDT descriptor.
 *
 * Every GDT descriptor is exactly 8 bytes. A segment selector is the
 * byte offset of a descriptor inside the GDT.
 */
#define GDT_DESCRIPTOR_SIZE 8

/**
 * @brief Index of the required null descriptor.
 */
#define GDT_NULL_INDEX 0

/**
 * @brief Index of the kernel code descriptor.
 */
#define GDT_KERNEL_CODE_INDEX 1

/**
 * @brief Index of the kernel data descriptor.
 */
#define GDT_KERNEL_DATA_INDEX 2

/**
 * @brief Selector for the null descriptor.
 */
#define GDT_NULL_SELECTOR (GDT_NULL_INDEX * GDT_DESCRIPTOR_SIZE)

/**
 * @brief Selector for the kernel code segment.
 */
#define GDT_KERNEL_CODE_SELECTOR (GDT_KERNEL_CODE_INDEX * GDT_DESCRIPTOR_SIZE)

/**
 * @brief Selector for the kernel data segment.
 */
#define GDT_KERNEL_DATA_SELECTOR (GDT_KERNEL_DATA_INDEX * GDT_DESCRIPTOR_SIZE)

/**
 * @brief Base address used for the flat memory model.
 */
#define GDT_FLAT_BASE 0x00000000

/**
 * @brief Segment limit used for a 4 GiB flat memory model.
 *
 * With 4 KiB granularity enabled, a limit of 0xFFFFF covers the 32-bit
 * address space.
 */
#define GDT_FLAT_LIMIT 0xFFFFF

/**
 * @brief Access byte flag indicating that the descriptor is present.
 */
#define GDT_ACCESS_PRESENT 0x80

/**
 * @brief Access byte flag for privilege ring 0.
 */
#define GDT_ACCESS_RING0 0x00

/**
 * @brief Access byte flag for a normal code/data descriptor.
 */
#define GDT_ACCESS_DESCRIPTOR 0x10

/**
 * @brief Access byte flag indicating an executable code segment.
 */
#define GDT_ACCESS_EXECUTABLE 0x08

/**
 * @brief Access byte flag allowing readable code or writable data.
 */
#define GDT_ACCESS_READABLE_WRITABLE 0x02

/**
 * @brief Access byte for the kernel code segment.
 */
#define GDT_ACCESS_KERNEL_CODE                                     \
  (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DESCRIPTOR | \
   GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READABLE_WRITABLE)

/**
 * @brief Access byte for the kernel data segment.
 */
#define GDT_ACCESS_KERNEL_DATA                                     \
  (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DESCRIPTOR | \
   GDT_ACCESS_READABLE_WRITABLE)

/**
 * @brief Granularity flag that makes the segment limit use 4 KiB blocks.
 */
#define GDT_GRANULARITY_4K 0x80

/**
 * @brief Granularity flag for a 32-bit protected mode segment.
 */
#define GDT_GRANULARITY_32BIT 0x40

/**
 * @brief Combined granularity flags for a 32-bit 4 KiB-granular segment.
 */
#define GDT_GRANULARITY_4K_32BIT (GDT_GRANULARITY_4K | GDT_GRANULARITY_32BIT)

/**
 * @brief Initializes and loads the kernel Global Descriptor Table.
 *
 * This function builds the kernel's GDT and loads it into the CPU.
 * After this runs, the kernel uses its own segment table instead of
 * relying on the one left behind by GRUB.
 */
void gdt_initialize(void);

#endif /* KERNEL_GDT_H */