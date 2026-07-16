#include <kernel/core/input.h>
#include <kernel/core/shell.h>
#include <kernel/drivers/keyboard.h>

/**
 * @file input.c
 * @brief Keyboard IRQ adapter for the active text input consumer.
 */

/**
 * @brief Handles one keyboard IRQ and forwards translated characters.
 */
void input_handle_keyboard_interrupt(void) {
  char ascii = '\0';

  if (keyboard_handle_interrupt(&ascii)) {
    shell_handle_character(ascii);
  }
}
