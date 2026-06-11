#include <kernel/vga.h>

/**
 * @file vga.c
 * @brief VGA text-mode terminal driver implementation.
 *
 * VGA text mode is normally 80 columns wide and 25 rows tall.
 * The text buffer begins at physical memory address 0xB8000.
 */

/**
 * @brief Current terminal row.
 */
static size_t terminal_row;

/**
 * @brief Current terminal column.
 */
static size_t terminal_column;

/**
 * @brief Active encoded VGA color byte.
 */
static uint8_t terminal_color;

/**
 * @brief Memory-mapped VGA text buffer.
 */
static volatile uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background) {
  return foreground | background << 4;
}

/**
 * @brief Builds one VGA text-mode buffer entry.
 *
 * @param character Character byte to display.
 * @param color Encoded VGA color byte.
 * @return Encoded 16-bit VGA text cell.
 *
 * Each VGA text cell is 2 bytes:
 * lower byte  = ASCII character
 * higher byte = color
 */
static uint16_t vga_entry(unsigned char character, uint8_t color) {
  return (uint16_t)character | (uint16_t)color << 8;
}

/**
 * @brief Computes the length of a null-terminated string.
 *
 * @param str String to measure.
 * @return Number of bytes before the null terminator.
 */
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

/**
 * @brief Advances the cursor to the beginning of the next row.
 */
static void terminal_newline(void) {
  terminal_column = 0;
  terminal_row++;

  /*
   * Simple Stage 2 behavior: if we reach the bottom, wrap back to the top.
   * Later, this can be replaced with real scrolling.
   */
  if (terminal_row >= VGA_HEIGHT) {
    terminal_row = 0;
  }
}

/**
 * @brief Moves the cursor to the beginning of the current row.
 */
static void terminal_carriage_return(void) { terminal_column = 0; }

/**
 * @brief Emits spaces until the cursor reaches the next tab stop.
 */
static void terminal_tab(void) {
  size_t spaces = TAB_WIDTH - (terminal_column % TAB_WIDTH);
  for (size_t i = 0; i < spaces; i++) {
    terminal_putchar(' ');
  }
}

/**
 * @brief Moves the cursor backward and clears the erased cell.
 */
static void terminal_backspace(void) {
  if (terminal_column > 0) {
    terminal_column--;
  } else if (terminal_row > 0) {
    terminal_row--;
    terminal_column = VGA_WIDTH - 1;
  } else {
    return;
  }

  size_t index = terminal_row * VGA_WIDTH + terminal_column;
  terminal_buffer[index] = vga_entry(' ', terminal_color);
}

/**
 * @brief Handles form-feed by clearing the terminal.
 */
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
