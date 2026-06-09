#include <kernel/vga.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
  Check if the compiler thinks you are targeting the wrong operating system.
*/
#if defined(__linux__)
#error \
    "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with an ix86-elf compiler"
#endif

void kernel_main(void) {
  /* Initialize terminal interface */
  terminal_initialize();

  terminal_writestring("Welcome to wigOSX\n");
  terminal_writestring("Stage 2: VGA text driver loaded.\n\n");

  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  terminal_writestring("VGA driver is working.\n");

  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
  terminal_writestring("Newline support is working.\n");

  terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));
  terminal_writestring("Color support is working.\n");

  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}