.section .text
.global gdt_flush
.type gdt_flush, @function

/**
 * @file gdt_flush.s
 * @brief Assembly helper for loading the i386 Global Descriptor Table.
 *
 * This file contains the low-level instructions needed to load the GDT.
 * C prepares the descriptor table, but assembly is required for lgdt and
 * for reloading the code segment register.
 */

/**
 * @brief Kernel code selector.
 *
 * This must match GDT_KERNEL_CODE_SELECTOR in
 * include/kernel/arch/i386/gdt.h.
 */
.set GDT_KERNEL_CODE_SELECTOR, 0x08

/**
 * @brief Kernel data selector.
 *
 * This must match GDT_KERNEL_DATA_SELECTOR in
 * include/kernel/arch/i386/gdt.h.
 */
.set GDT_KERNEL_DATA_SELECTOR, 0x10

/**
 * @brief Loads the GDT and reloads all segment registers.
 *
 * @param gdt_ptr_address Address of the GDT pointer structure.
 *
 * Stack layout on entry:
 * - 0(%esp): return address
 * - 4(%esp): first argument, address of struct gdt_pointer
 *
 * This function:
 * - loads the GDTR register using lgdt
 * - reloads DS, ES, FS, GS, and SS with the kernel data selector
 * - reloads CS using a far jump
 */
gdt_flush:
  mov 4(%esp), %eax

  lgdt (%eax)

  mov $GDT_KERNEL_DATA_SELECTOR, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss

  ljmp $GDT_KERNEL_CODE_SELECTOR, $gdt_flush_done

gdt_flush_done:
  ret

.size gdt_flush, . - gdt_flush
