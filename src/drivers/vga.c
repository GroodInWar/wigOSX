#include <kernel/arch/i386/io.h>
#include <kernel/drivers/vga.h>

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
 * @brief First visible VGA cell in the hardware viewport.
 *
 * This lets the terminal scroll by changing the VGA display start address
 * instead of copying the whole visible screen.
 */
static size_t terminal_view_start;

/**
 * @brief Active encoded VGA color byte.
 */
static uint8_t terminal_color;

/**
 * @brief Memory-mapped VGA text buffer.
 */
static volatile uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

/**
 * @brief Wraps a VGA text-memory cell index into the valid text buffer range.
 *
 * @param index Unbounded VGA text-cell index.
 * @return Equivalent index within VGA text memory.
 */
static size_t terminal_wrap_index(size_t index) {
  return index % VGA_TEXT_MEMORY_CELLS;
}

/**
 * @brief Converts visible terminal coordinates into a physical VGA memory
 * index.
 *
 * The visible screen begins at terminal_view_start. Scrolling changes that
 * starting cell instead of copying the screen upward.
 *
 * @param row Visible zero-based terminal row.
 * @param column Visible zero-based terminal column.
 * @return Physical cell index within VGA text memory.
 */
static size_t terminal_get_buffer_index(size_t row, size_t column) {
  return terminal_wrap_index(terminal_view_start + row * VGA_WIDTH + column);
}

/**
 * @brief Programs the VGA CRTC start address.
 *
 * The address is measured in text cells, not bytes.
 *
 * @param cell_offset First text cell to display in the hardware viewport.
 */
static void terminal_set_hardware_view_start(size_t cell_offset) {
  uint16_t address = (uint16_t)cell_offset;

  outb(VGA_CRTC_INDEX_PORT, VGA_CRTC_START_ADDRESS_HIGH);
  outb(VGA_CRTC_DATA_PORT, (address >> 8) & 0xFF);

  outb(VGA_CRTC_INDEX_PORT, VGA_CRTC_START_ADDRESS_LOW);
  outb(VGA_CRTC_DATA_PORT, address & 0xFF);
}

/**
 * @brief Builds one VGA text-mode buffer entry.
 *
 * @param character Character byte to display.
 * @param color Encoded VGA color byte.
 * @return Encoded 16-bit VGA text cell.
 *
 * Each VGA text cell stores the character in the low byte and the color in
 * the high byte.
 */
static uint16_t vga_entry(unsigned char character, uint8_t color) {
  return (uint16_t)character | (uint16_t)color << 8;
}

uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background) {
  return foreground | background << 4;
}

/**
 * @brief Clears one visible terminal row.
 *
 * @param row Visible zero-based row to clear.
 */
static void terminal_clear_row(size_t row) {
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    size_t index = terminal_get_buffer_index(row, x);
    terminal_buffer[index] = vga_entry(' ', terminal_color);
  }
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
  terminal_view_start = 0;
  terminal_set_hardware_view_start(terminal_view_start);

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    terminal_clear_row(y);
  }

  terminal_row = 0;
  terminal_column = 0;
}

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_view_start = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

  terminal_clear();
}

void terminal_setcolor(uint8_t color) { terminal_color = color; }

/**
 * @brief Scrolls the terminal by moving the VGA hardware viewport.
 *
 * This avoids copying every visible row. The display start address moves down
 * by one row, and only the new bottom row is cleared.
 */
static void terminal_scroll_up(void) {
  terminal_view_start = terminal_wrap_index(terminal_view_start + VGA_WIDTH);
  terminal_set_hardware_view_start(terminal_view_start);

  terminal_clear_row(VGA_HEIGHT - 1);
}

/**
 * @brief Advances the cursor to the beginning of the next row.
 */
static void terminal_newline(void) {
  terminal_column = 0;

  if (terminal_row + 1 >= VGA_HEIGHT) {
    terminal_scroll_up();
    terminal_row = VGA_HEIGHT - 1;
    return;
  }

  terminal_row++;
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

  size_t index = terminal_get_buffer_index(terminal_row, terminal_column);
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

  size_t index = terminal_get_buffer_index(terminal_row, terminal_column);
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
