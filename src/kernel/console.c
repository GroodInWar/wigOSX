#include <kernel/core/console.h>
#include <kernel/drivers/vga.h>
#include <stddef.h>

/**
 * @file console.c
 * @brief Console service backed by the VGA terminal driver.
 */

/**
 * @brief Tracks whether console output can be used.
 */
static bool console_initialized = false;

/**
 * @brief Initializes the terminal backend and marks the console ready.
 */
void console_initialize(void) {
  terminal_initialize();
  console_initialized = true;
}

/**
 * @brief Reports whether the console has been initialized.
 */
bool console_is_initialized(void) { return console_initialized; }

/**
 * @brief Writes one character to the initialized console.
 */
void console_putchar(char character) {
  if (!console_initialized) {
    return;
  }

  terminal_putchar(character);
}

/**
 * @brief Writes one null-terminated string to the initialized console.
 */
void console_writestring(const char* text) {
  if (!console_initialized || text == NULL) {
    return;
  }

  terminal_writestring(text);
}

/**
 * @brief Clears the initialized console display.
 */
void console_clear(void) {
  if (!console_initialized) {
    return;
  }

  terminal_clear();
}
