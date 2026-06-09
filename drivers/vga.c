#include <kernel/vga.h>

/*
 * VGA text mode is normally 80 columns wide and 25 rows tall.
 * The text buffer begins at physical memory address 0xB8000.
 */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static volatile uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background) {
  return foreground | background << 4;
}

/*
 * Each VGA text cell is 2 bytes:
 * lower byte  = ASCII character
 * higher byte = color
 */
static uint16_t vga_entry(unsigned char character, uint8_t color) {
  return (uint16_t)character | (uint16_t)color << 8;
}

static size_t terminal_strlen(const char* str) {
  size_t length = 0;

  while (str[length] != '\0') {
    length++;
  }

  return length;
}

void terminal_clear(void) {
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }

  terminal_row = 0;
  terminal_column = 0;
}

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

  terminal_clear();
}

void terminal_setcolor(uint8_t color) { terminal_color = color; }

static void terminal_newline(void) {
  terminal_column = 0;
  terminal_row++;

  /*
   * Simple Stage 2 behavior:
   * If we reach the bottom, wrap back to the top.
   * Later, we can replace this with real scrolling.
   */
  if (terminal_row >= VGA_HEIGHT) {
    terminal_row = 0;
  }
}

static void terminal_carriage_return(void) { terminal_column = 0; }

static void terminal_tab(void) {
  terminal_column = (terminal_column + 8) & ~(8 - 1);
}

static void terminal_backspace(void) {
  if (terminal_column > 0) {
    terminal_column--;
  }
}

static void terminal_form_feed(void) { terminal_clear(); }

void terminal_putchar(char c) {
  switch (c) {
    case '\n':
      terminal_newline();
      return;
    case '\r':
      terminal_carriage_return();
      return;
    case '\t':
      terminal_tab();
      return;
    case '\b':
      terminal_backspace();
      return;
    case '\f':
      terminal_form_feed();
      return;
    default:
      break;
  }

  size_t index = terminal_row * VGA_WIDTH + terminal_column;
  terminal_buffer[index] = vga_entry(c, terminal_color);

  terminal_column++;

  if (terminal_column >= VGA_WIDTH) {
    terminal_newline();
  }
}

void terminal_write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    terminal_putchar(data[i]);
  }
}

void terminal_writestring(const char* data) {
  terminal_write(data, terminal_strlen(data));
}
