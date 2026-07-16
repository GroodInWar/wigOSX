#include <kernel/arch/i386/cpu.h>
#include <kernel/arch/i386/gdt.h>
#include <kernel/arch/i386/idt.h>
#include <kernel/arch/i386/pic.h>
#include <kernel/core/console.h>
#include <kernel/core/input.h>
#include <kernel/core/interrupts.h>
#include <kernel/core/kernel.h>
#include <kernel/core/log.h>
#include <kernel/core/memory.h>
#include <kernel/core/panic.h>
#include <kernel/core/shell.h>
#include <kernel/core/status.h>
#include <kernel/core/version.h>
#include <kernel/drivers/pit.h>
#include <kernel/mm/pmm.h>
#include <kernel/mm/vmm.h>
#include <stdint.h>

/**
 * @file kernel.c
 * @brief Main C entry point for wigOSX.
 */

/**
 * @brief Verifies that the configured compiler is not targeting the host OS.
 *
 * Check if the compiler thinks you are targeting the wrong operating system.
 */
#if defined(__linux__)
#error \
    "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/**
 * @brief Verifies that the kernel is being built for 32-bit x86.
 */
#if !defined(__i386__)
#error "This kernel needs to be compiled with an i686-elf compiler"
#endif

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/**
 * @brief Panics if a subsystem initialization status is not successful.
 */
static void kernel_require(kernel_status_t status, const char* panic_message) {
  if (!kernel_status_is_ok(status)) {
    kernel_panic(panic_message);
  }
}

/**
 * @brief Main kernel entry point called from the boot assembly.
 *
 * Validates early boot state, initializes visible terminal output, starts
 * serial logging, initializes descriptor tables and hardware interrupts, then
 * starts the kernel shell.
 */
void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info_address) {
  cpu_disable_interrupts();
  console_initialize();

  if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    kernel_panic("Invalid Multiboot bootloader magic");
  }

  if (multiboot_info_address == 0) {
    kernel_panic("Multiboot information address is zero");
  }

  kernel_status_t log_status = klog_initialize();

  if (log_status == KERNEL_STATUS_UNAVAILABLE) {
    console_writestring(
        "[WARNING] Serial logging unavailable; using VGA only.\n");
  }

  klog_writestring("wigOSX ");
  klog_writestring(WIGOSX_VERSION_STRING);
  klog_putchar('\n');

  klog_writestring("Initializing normalized memory map...\n");
  kernel_require(memory_initialize(multiboot_info_address),
                 "Memory-map initialization failed");
  memory_print_summary();

  klog_writestring("Initializing physical memory manager...\n");
  kernel_require(pmm_initialize(multiboot_info_address),
                 "Physical memory manager initialization failed");
  pmm_print_summary();

  klog_writestring("Initializing GDT...\n");
  gdt_initialize();

  klog_writestring("Initializing virtual memory manager...\n");
  kernel_require(vmm_initialize(),
                 "Virtual memory manager initialization failed");
  vmm_print_summary();

  klog_writestring("Initializing interrupt services...\n");
  interrupts_initialize();
  idt_initialize();

  pic_remap();
  pic_mask_all();

  if (!interrupts_register_irq(0, pit_handle_interrupt)) {
    kernel_panic("Failed to register PIT IRQ handler");
  }

  if (!interrupts_register_irq(1, input_handle_keyboard_interrupt)) {
    kernel_panic("Failed to register keyboard IRQ handler");
  }

  if (!pit_initialize(100)) {
    kernel_panic("PIT initialization failed");
  }

  /*
   * Initialize the character consumer before keyboard interrupts can arrive.
   */
  shell_initialize();

  if (!pic_unmask_irq(0)) {
    kernel_panic("Failed to unmask PIT IRQ0");
  }

  if (!pic_unmask_irq(1)) {
    kernel_panic("Failed to unmask keyboard IRQ1");
  }

  /*
   * No initialization or output should occur after this point unless it is
   * safe to run concurrently with interrupt handlers.
   */
  cpu_enable_interrupts();

  while (1) {
    cpu_halt();
  }
}
