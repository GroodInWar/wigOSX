#ifndef KERNEL_VGA_H
#define KERNEL_VGA_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file vga.h
 * @brief Public interface for the VGA text-mode terminal driver.
 *
 * The terminal driver writes directly to the VGA text buffer and provides
 * simple character, string, color, and screen-clear operations for early
 * kernel output.
 */

/**
 * @brief Number of columns in the VGA text-mode screen.
 */
#define VGA_WIDTH 80

/**
 * @brief Number of rows in the VGA text-mode screen.
 */
#define VGA_HEIGHT 25

/**
 * @brief Physical address of the VGA text buffer.
 */
#define VGA_MEMORY 0xB8000

/**
 * @brief Number of spaces represented by a tab character.
 */
#define TAB_WIDTH 4

/**
 * @brief Total VGA text memory size in bytes.
 *
 * Color text mode usually has 32 KiB available from 0xB8000 to 0xBFFFF.
 */
#define VGA_TEXT_MEMORY_BYTES 32768

/**
 * @brief Number of VGA text cells available in text memory.
 *
 * Each text cell is 2 bytes: one character byte and one color byte.
 */
#define VGA_TEXT_MEMORY_CELLS (VGA_TEXT_MEMORY_BYTES / sizeof(uint16_t))

/**
 * @brief VGA CRTC index and data ports.
 *
 * These are used to change the visible start address of the VGA text viewport.
 */
#define VGA_CRTC_INDEX_PORT 0x3D4
#define VGA_CRTC_DATA_PORT 0x3D5

/**
 * @brief CRTC registers for the display start address.
 */
#define VGA_CRTC_START_ADDRESS_HIGH 0x0C
#define VGA_CRTC_START_ADDRESS_LOW 0x0D

/**
 * @brief VGA text-mode color values.
 *
 * VGA text mode supports 16 basic colors.
 * These values become part of the byte that controls text color.
 */
enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

/**
 * @brief Combines foreground and background colors into one VGA color byte.
 *
 * @param foreground Text foreground color.
 * @param background Text background color.
 * @return Encoded VGA color byte.
 *
 * The foreground color occupies the low nibble and the background color
 * occupies the high nibble.
 */
uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background);

/**
 * @brief Initializes the terminal state and clears the screen.
 *
 * The initial color is light grey text on a black background, and the cursor
 * is reset to the top-left corner.
 */
void terminal_initialize(void);

/**
 * @brief Clears the visible VGA screen and resets the cursor to the top-left.
 *
 * The current terminal color is used for the blank cells.
 */
void terminal_clear(void);

/**
 * @brief Sets the active color used for subsequent terminal output.
 *
 * @param color Encoded VGA color byte.
 */
void terminal_setcolor(uint8_t color);

/**
 * @brief Writes one character at the current cursor position.
 *
 * @param c Character or supported control character to write.
 *
 * Newline, carriage return, tab, backspace, and form-feed characters are
 * handled as terminal controls. Reaching the right or bottom edge wraps or
 * scrolls the visible terminal as needed.
 */
void terminal_putchar(char c);

/**
 * @brief Writes a fixed-size byte range to the terminal.
 *
 * @param data Pointer to the bytes to write.
 * @param size Number of bytes to write.
 *
 * The byte range does not need to be null-terminated. @p data must point to at
 * least @p size readable bytes.
 */
void terminal_write(const char* data, size_t size);

/**
 * @brief Writes a null-terminated string to the terminal.
 *
 * @param data Null-terminated string to write.
 *
 * @p data must not be NULL.
 */
void terminal_writestring(const char* data);

#endif /* KERNEL_VGA_H */
