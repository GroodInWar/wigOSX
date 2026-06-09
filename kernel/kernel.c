#include <kernel/serial.h>
#include <kernel/test.h>
#include <kernel/vga.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Check if the compiler thinks you are targeting the wrong operating system.
 */
#if defined(__linux__)
#error \
    "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This kernel currently targets 32-bit x86. */
#if !defined(__i386__)
#error "This kernel needs to be compiled with an i686-elf compiler"
#endif

void kernel_main(void) { 
  terminal_initialize();

  terminal_writestring("Welcome to wigOSX 0.003!\n");
  terminal_writestring("Initializing serial logging...\n");

  serial_initialize();

  if (serial_is_initialized()) {
    serial_writestring("Serial logging initialized successfully.\n");
    serial_writestring("[wigOSX] Stage 3: Serial Logging Initialized\n");
  } else {
    serial_writestring("Serial logging failed to initialize.\n");
  }

  serial_writestring("[wigOSX] Running VGA visual tests...\n");
  run_vga_tests(); 
  serial_writestring("[wigOSX] VGA visual tests completed.\n");
}