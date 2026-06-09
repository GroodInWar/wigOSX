#include <kernel/test.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Check if the compiler thinks you are targeting the wrong operating system.
 */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This kernel currently targets 32-bit x86. */
#if !defined(__i386__)
#error "This kernel needs to be compiled with an i686-elf compiler"
#endif

void kernel_main(void) {
  run_vga_tests();
}