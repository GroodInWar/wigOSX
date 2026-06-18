#include <kernel/arch/i386/cpu.h>
#include <kernel/arch/i386/gdt.h>
#include <kernel/arch/i386/idt.h>
#include <kernel/arch/i386/pic.h>
#include <kernel/core/kernel.h>
#include <kernel/core/shell.h>
#include <kernel/core/test.h>
#include <kernel/drivers/pit.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <stdbool.h>
#include <stddef.h>
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
 * @brief Halts the CPU forever after a fatal early-kernel error.
 */
static void kernel_halt_forever(void) {
  while (1) {
    cpu_halt();
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
  /* Bring up VGA first so every later initialization step can report status. */
  terminal_initialize();

  terminal_writestring("Checking Multiboot information...\n");

  if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    terminal_writestring("FATAL: invalid Multiboot magic.\n");
    kernel_halt_forever();
  }

  if (multiboot_info_address == 0) {
    terminal_writestring("FATAL: Multiboot info address is zero.\n");
    kernel_halt_forever();
  }

  terminal_writestring("Multiboot information looks valid.\n");

  terminal_writestring("Initializing serial logging...\n");

  if (serial_initialize()) {
    terminal_writestring("Serial logging initialized successfully.\n");
    serial_writestring("[wigOSX] Stage 3: Serial logging initialized.\n");
  } else {
    terminal_writestring(
        "Serial logging unavailable; continuing without serial.\n");
  }

  terminal_writestring("Initializing GDT...\n");
  serial_writestring("[wigOSX] Stage 4: Initializing GDT...\n");

  /* Load a kernel-owned flat GDT before installing interrupt descriptors. */
  gdt_initialize();

  terminal_writestring("GDT initialized successfully.\n");
  serial_writestring("[wigOSX] Stage 4: GDT initialized successfully.\n");

  terminal_writestring("Initializing IDT...\n");
  serial_writestring("[wigOSX] Stage 5: Initializing IDT...\n");

  /* Install exception and IRQ gates, but keep maskable interrupts disabled. */
  idt_initialize();

  terminal_writestring("IDT initialized successfully.\n");
  serial_writestring("[wigOSX] Stage 5: IDT initialized successfully.\n");

  terminal_writestring("Initializing PIC...\n");
  serial_writestring("[wigOSX] Stage 6: Initializing PIC...\n");

  /*
   * Move hardware IRQs out of the CPU exception range, then enable:
   * - IRQ0 for the PIT timer
   * - IRQ1 for the PS/2 keyboard
   */
  pic_remap();
  pic_mask_all();
  if (!pic_unmask_irq(0)) {
    terminal_writestring("FATAL: failed to unmask PIT IRQ0.\n");
    kernel_halt_forever();
  }

  if (!pic_unmask_irq(1)) {
    terminal_writestring("FATAL: failed to unmask keyboard IRQ1.\n");
    kernel_halt_forever();
  }

  terminal_writestring("PIC initialized successfully.\n");
  serial_writestring("[wigOSX] Stage 6: PIC initialized.\n");
  serial_writestring("[wigOSX] Stage 7: IRQ1 enabled.\n");

  terminal_writestring("Initializing PIT...\n");
  serial_writestring("[wigOSX] Stage 6: Initializing PIT at 100 Hz...\n");

  /* Program the timer before allowing the PIC to deliver IRQ0 to the CPU. */
  pit_initialize(100);
  if (!pit_initialize(100)) {
    terminal_writestring("FATAL: PIT initialization failed.\n");
    kernel_halt_forever();
  }
  terminal_writestring("PIT initialized successfully.\n");
  serial_writestring("[wigOSX] Stage 6: PIT initialized at 100 Hz.\n");

  cpu_enable_interrupts();

  terminal_writestring("Hardware interrupts enabled.\n");
  serial_writestring("[wigOSX] Interrupts enabled.\n");

  /*
   * The VGA visual suite is useful during terminal-driver work. It is left
   * here as an easy manual test hook without running on every boot.
   */
  // terminal_writestring("Running VGA visual tests...\n");
  // serial_writestring("[wigOSX] Running VGA visual tests...\n");

  // run_vga_tests();
  // terminal_clear();

  // serial_writestring("[wigOSX] VGA visual tests completed.\n");
  terminal_writestring("Welcome to wigOSX 0.010!\n");
  terminal_writestring("Stage 10: VGA terminal scrolling enabled.\n");
  terminal_writestring("Starting kernel shell...\n");

  serial_writestring("[wigOSX] Stage 10: VGA terminal scrolling enabled.\n");
  shell_initialize();

  /* Sleep until interrupts arrive instead of burning CPU in a spin loop. */
  while (1) {
    cpu_halt();
  }
}
