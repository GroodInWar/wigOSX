/**
 * @file boot.s
 * @brief Multiboot header and early i386 kernel entry point.
 *
 * This assembly file is the first code entered by the bootloader. It exposes
 * the Multiboot header, installs an initial stack, calls kernel_main(), and
 * halts the CPU if the kernel returns.
 */

/**
 * @brief Multiboot flag requesting page-aligned modules.
 */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */

/**
 * @brief Multiboot flag requesting a memory map.
 */
.set MEMINFO,  1<<1             /* provide memory map */

/**
 * @brief Combined Multiboot flags used by the kernel header.
 */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */

/**
 * @brief Multiboot magic value that identifies this file as a kernel image.
 */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */

/**
 * @brief Multiboot checksum that validates the magic and flags.
 */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/**
 * @brief Multiboot header recognized by GRUB.
 *
 * These values are documented in the Multiboot standard. The bootloader
 * searches for this signature in the first 8 KiB of the kernel file, aligned
 * at a 32-bit boundary. The signature is in its own section so the linker can
 * force it to be within the first 8 KiB of the kernel image.
 */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/**
 * @brief Early boot stack storage.
 *
 * The Multiboot standard does not define the value of ESP, so the kernel
 * provides its own stack. The stack grows downward on x86 and is aligned to
 * 16 bytes to satisfy the System V ABI assumptions used by the compiler.
 */
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/**
 * @brief Kernel entry point named by the linker script.
 *
 * The bootloader jumps here after loading the kernel image.
 */
.section .text
.global _start
.type _start, @function
_start:
  /*
   * The bootloader has loaded us into 32-bit protected mode on an x86
   * machine. Interrupts and paging are disabled, and the processor state is
   * otherwise defined by the Multiboot standard.
   */
  
  /* Set ESP to the top of the early boot stack before entering C code. */
  mov $stack_top, %esp

  /*
   * Additional processor setup such as GDT, paging, or floating-point support
   * can be placed here before the high-level kernel is entered.
   */

  /* Enter the high-level kernel with the stack alignment expected by the ABI. */
	call kernel_main

  /* Halt forever if kernel_main() returns. */
	cli
1:	hlt
	jmp 1b

/**
 * @brief Emits the size metadata for the _start symbol.
 */
.size _start, . - _start
