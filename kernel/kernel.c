#include <kernel/gdt.h>
#include <kernel/serial.h>
#include <kernel/test.h>
#include <kernel/vga.h>
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

/**
 * @brief Main kernel entry point called from the boot assembly.
 *
 * Initializes visible terminal output, starts serial logging, and runs the
 * current VGA visual test suite.
 */
void kernel_main(void) {
  terminal_initialize();

  terminal_writestring("Initializing serial logging...\n");

  serial_initialize();

  if (serial_is_initialized()) {
    terminal_writestring("Serial logging initialized successfully.\n");
    serial_writestring("[wigOSX] Stage 3: Serial Logging Initialized\n");
  } else {
    terminal_writestring("Serial logging failed to initialize.\n");
  }

  terminal_writestring("Initializing GDT...\n");
  serial_writestring("[wigOSX] Stage 4: Initializing GDT...\n");
  
  gdt_initialize();

  terminal_writestring("GDT initialized successfully.\n");
  serial_writestring("[wigOSX] Stage 4: GDT initialized successfully.\n");
  
  terminal_writestring("Running VGA visual tests...\n");
  serial_writestring("[wigOSX] Running VGA visual tests...\n");

  run_vga_tests();
  terminal_clear();

  serial_writestring("[wigOSX] VGA visual tests completed.\n");
  terminal_writestring("Welcome to wigOSX 0.004!\n");
}
