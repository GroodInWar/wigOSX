#include <kernel/arch/i386/io.h>
#include <kernel/core/shell.h>
#include <kernel/drivers/keyboard.h>
#include <kernel/drivers/serial.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @file keyboard.c
 * @brief PS/2 keyboard driver for IRQ1.
 *
 * Stage 9 adds basic keyboard modifier support. The driver now tracks Shift
 * and Caps Lock, translates Set 1 scancodes into ASCII, and passes completed
 * characters to the shell.
 *
 * This version still ignores Ctrl, Alt, function keys, and extended scancodes.
 */

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_RELEASED_MASK 0x80
#define KEYBOARD_SCANCODE_TABLE_SIZE 128

#define SCANCODE_LEFT_SHIFT 0x2A
#define SCANCODE_RIGHT_SHIFT 0x36
#define SCANCODE_CAPS_LOCK 0x3A

static bool keyboard_shift_pressed = false;
static bool keyboard_caps_lock_enabled = false;

static const char keyboard_normal_map[KEYBOARD_SCANCODE_TABLE_SIZE] = {
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

static const char keyboard_shift_map[KEYBOARD_SCANCODE_TABLE_SIZE] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#',  [0x05] = '$',  [0x06] = '%',
    [0x07] = '^', [0x08] = '&', [0x09] = '*',  [0x0A] = '(',  [0x0B] = ')',
    [0x0C] = '_', [0x0D] = '+', [0x0E] = '\b', [0x0F] = '\t',

    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E',  [0x13] = 'R',  [0x14] = 'T',
    [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',  [0x18] = 'O',  [0x19] = 'P',
    [0x1A] = '{', [0x1B] = '}', [0x1C] = '\n',

    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D',  [0x21] = 'F',  [0x22] = 'G',
    [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',  [0x26] = 'L',  [0x27] = ':',
    [0x28] = '"', [0x29] = '~',

    [0x2B] = '|', [0x2C] = 'Z', [0x2D] = 'X',  [0x2E] = 'C',  [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M',  [0x33] = '<',  [0x34] = '>',
    [0x35] = '?', [0x39] = ' ',
};

static bool keyboard_is_letter_scancode(uint8_t scancode) {
  return (scancode >= 0x10 && scancode <= 0x19) ||
         (scancode >= 0x1E && scancode <= 0x26) ||
         (scancode >= 0x2C && scancode <= 0x32);
}

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

static void keyboard_handle_modifier_press(uint8_t scancode) {
  if (scancode == SCANCODE_LEFT_SHIFT || scancode == SCANCODE_RIGHT_SHIFT) {
    keyboard_shift_pressed = true;
  } else if (scancode == SCANCODE_CAPS_LOCK) {
    keyboard_caps_lock_enabled = !keyboard_caps_lock_enabled;

    if (keyboard_caps_lock_enabled) {
      serial_writestring("[wigOSX] Keyboard: Caps Lock enabled\n");
    } else {
      serial_writestring("[wigOSX] Keyboard: Caps Lock disabled\n");
    }
  }
}

static void keyboard_handle_modifier_release(uint8_t scancode) {
  if (scancode == SCANCODE_LEFT_SHIFT || scancode == SCANCODE_RIGHT_SHIFT) {
    keyboard_shift_pressed = false;
  }
}

static char keyboard_translate_scancode(uint8_t scancode) {
  if (scancode >= KEYBOARD_SCANCODE_TABLE_SIZE) {
    return '\0';
  }

  if (keyboard_is_letter_scancode(scancode)) {
    bool uppercase = keyboard_shift_pressed ^ keyboard_caps_lock_enabled;

    if (uppercase) {
      return keyboard_shift_map[scancode];
    }

    return keyboard_normal_map[scancode];
  }

  if (keyboard_shift_pressed) {
    return keyboard_shift_map[scancode];
  }

  return keyboard_normal_map[scancode];
}

void keyboard_handle_interrupt(void) {
  uint8_t raw_scancode = inb(KEYBOARD_DATA_PORT);
  bool released = (raw_scancode & KEYBOARD_RELEASED_MASK) != 0;
  uint8_t scancode = raw_scancode & ~KEYBOARD_RELEASED_MASK;

  if (released) {
    keyboard_handle_modifier_release(scancode);
    return;
  }

  keyboard_handle_modifier_press(scancode);

  if (scancode == SCANCODE_LEFT_SHIFT || scancode == SCANCODE_RIGHT_SHIFT ||
      scancode == SCANCODE_CAPS_LOCK) {
    return;
  }

  char ascii = keyboard_translate_scancode(scancode);

  if (ascii == '\0') {
    serial_writestring("[wigOSX] Keyboard: unsupported scancode\n");
    return;
  }

  shell_handle_character(ascii);
  keyboard_log_ascii(ascii);
}