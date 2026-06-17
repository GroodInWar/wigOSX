#include <kernel/arch/i386/io.h>
#include <kernel/core/shell.h>
#include <kernel/drivers/keyboard.h>
#include <kernel/drivers/serial.h>
#include <stdint.h>

/**
 * @file keyboard.c
 * @brief Minimal PS/2 keyboard driver for IRQ1.
 *
 * Stage 8 reads keyboard scancodes from the PS/2 data port, translates
 * basic Set 1 scancodes into ASCII, and passes characters to the shell.
 *
 * This first version supports basic unshifted US QWERTY keys.
 * It ignores key-release events, Shift, Ctrl, Alt, Caps Lock, and extended
 * scancodes for now.
 */

/**
 * @brief PS/2 keyboard data port.
 *
 * The keyboard places one scancode byte here when IRQ1 fires.
 */
#define KEYBOARD_DATA_PORT 0x60

/**
 * @brief Bit set in Set 1 scancodes when a key is released.
 */
#define KEYBOARD_RELEASED_MASK 0x80

/**
 * @brief Number of entries in the basic scancode table.
 */
#define KEYBOARD_SCANCODE_TABLE_SIZE 128

/**
 * @brief Maps basic Set 1 keyboard scancodes to ASCII.
 *
 * Unsupported entries stay as '\0'.
 */
static const char keyboard_scancode_to_ascii[KEYBOARD_SCANCODE_TABLE_SIZE] = {
    [0x02] = '1',  [0x03] = '2', [0x04] = '3',  [0x05] = '4',  [0x06] = '5',
    [0x07] = '6',  [0x08] = '7', [0x09] = '8',  [0x0A] = '9',  [0x0B] = '0',
    [0x0C] = '-',  [0x0D] = '=', [0x0E] = '\b', [0x0F] = '\t',

    [0x10] = 'q',  [0x11] = 'w', [0x12] = 'e',  [0x13] = 'r',  [0x14] = 't',
    [0x15] = 'y',  [0x16] = 'u', [0x17] = 'i',  [0x18] = 'o',  [0x19] = 'p',
    [0x1A] = '[',  [0x1B] = ']', [0x1C] = '\n',

    [0x1E] = 'a',  [0x1F] = 's', [0x20] = 'd',  [0x21] = 'f',  [0x22] = 'g',
    [0x23] = 'h',  [0x24] = 'j', [0x25] = 'k',  [0x26] = 'l',  [0x27] = ';',
    [0x28] = '\'', [0x29] = '`',

    [0x2B] = '\\', [0x2C] = 'z', [0x2D] = 'x',  [0x2E] = 'c',  [0x2F] = 'v',
    [0x30] = 'b',  [0x31] = 'n', [0x32] = 'm',  [0x33] = ',',  [0x34] = '.',
    [0x35] = '/',  [0x39] = ' ',
};

/**
 * @brief Logs a keyboard character to the serial port.
 *
 * @param ascii ASCII character produced by the keyboard driver.
 */
static void keyboard_log_ascii(char ascii) {
  if (ascii == '\n') {
    serial_writestring("[wigOSX] Keyboard: ENTER\n");
  } else if (ascii == '\b') {
    serial_writestring("[wigOSX] Keyboard: BACKSPACE\n");
  } else if (ascii == '\t') {
    serial_writestring("[wigOSX] Keyboard: TAB\n");
  } else {
    serial_writestring("[wigOSX] Keyboard: ");
    serial_putchar(ascii);
    serial_putchar('\n');
  }
}

/**
 * @brief Handles one PS/2 keyboard interrupt.
 *
 * Reads one scancode from port 0x60, ignores key releases, translates supported
 * key presses to ASCII, and writes the result to VGA and serial output.
 */
void keyboard_handle_interrupt(void) {
  uint8_t scancode = inb(KEYBOARD_DATA_PORT);

  /*
   * In Set 1, release events have bit 7 set.
   * Stage 7 only cares about key presses.
   */
  if ((scancode & KEYBOARD_RELEASED_MASK) != 0) {
    return;
  }

  if (scancode >= KEYBOARD_SCANCODE_TABLE_SIZE) {
    serial_writestring("[wigOSX] Keyboard: scancode out of range\n");
    return;
  }

  char ascii = keyboard_scancode_to_ascii[scancode];

  if (ascii == '\0') {
    serial_writestring("[wigOSX] Keyboard: unsupported scancode\n");
    return;
  }

  shell_handle_character(ascii);
  keyboard_log_ascii(ascii);
}