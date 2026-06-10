/**
 * @file linker.ld
 * @brief Linker script for the wigOSX i386 kernel image.
 *
 * The script places the Multiboot header early in the image, arranges kernel
 * sections on 4 KiB boundaries, and names _start as the entry point.
 */

/**
 * @brief Symbol where the bootloader begins executing the kernel.
 */
ENTRY(_start)

/**
 * @brief Output section layout for the final kernel image.
 */
SECTIONS
{
  /**
   * @brief Kernel load address.
   *
   * The kernel is linked at 2 MiB, which is a safer modern load point than the
   * traditional 1 MiB address on some firmware layouts.
   */
  . = 2M;

  /**
   * @brief Executable code and the Multiboot header.
   */
  .text BLOCK(4K) : ALIGN(4K) 
  {
    *(.multiboot)
    *(.text)
  }

  /**
   * @brief Read-only data.
   */
  .rodata BLOCK(4K) : ALIGN(4K)
  {
    *(.rodata)
  }

  /**
   * @brief Initialized read-write data.
   */
  .data BLOCK(4K) : ALIGN(4K)
  {
    *(.data)
  }

  /**
   * @brief Uninitialized read-write data and the early boot stack.
   */
  .bss BLOCK(4K) : ALIGN(4K)
  {
    *(COMMON)
    *(.bss)
  }

  /**
   * Additional compiler-generated sections can be added here as needed.
   */
}
