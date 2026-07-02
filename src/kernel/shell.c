#include <kernel/core/memory.h>
#include <kernel/core/shell.h>
#include <kernel/core/version.h>
#include <kernel/drivers/pit.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <kernel/mm/pmm.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file shell.c
 * @brief Tiny early kernel shell.
 *
 * The shell receives already-translated ASCII characters from the keyboard
 * driver. It stores one command line, echoes typed characters, and executes
 * simple built-in commands when Enter is pressed.
 */

/**
 * @brief Maximum number of command characters accepted before Enter.
 *
 * The extra byte is for the null terminator.
 */
#define SHELL_BUFFER_SIZE 64

/**
 * @brief Prompt shown before each command.
 */
#define SHELL_PROMPT "wigOSX> "

/**
 * @brief Current command input buffer.
 */
static char shell_buffer[SHELL_BUFFER_SIZE];

/**
 * @brief Number of characters currently stored in shell_buffer.
 */
static size_t shell_buffer_length = 0;

/**
 * @brief Prints the shell prompt.
 */
static void shell_print_prompt(void) { terminal_writestring(SHELL_PROMPT); }

/**
 * @brief Returns true when two null-terminated strings are equal.
 *
 * This avoids depending on a hosted C library strcmp().
 */
static bool shell_strings_equal(const char* left, const char* right) {
  size_t index = 0;

  while (left[index] != '\0' && right[index] != '\0') {
    if (left[index] != right[index]) {
      return false;
    }

    index++;
  }

  return left[index] == '\0' && right[index] == '\0';
}

/**
 * @brief Returns true when the command line has no visible characters.
 */
static bool shell_command_is_empty(const char* command) {
  return command[0] == '\0';
}

/**
 * @brief Prints an unsigned 32-bit integer in decimal.
 *
 * This is intentionally small and local to the shell for now. Later we can move
 * integer formatting into a shared kernel string/stdio helper.
 */
static void shell_print_uint32(uint32_t value) {
  char digits[10];
  size_t digit_count = 0;

  if (value == 0) {
    terminal_putchar('0');
    return;
  }

  while (value > 0) {
    digits[digit_count] = '0' + (value % 10);
    value = value / 10;
    digit_count++;
  }

  while (digit_count > 0) {
    digit_count--;
    terminal_putchar(digits[digit_count]);
  }
}

/**
 * @brief Implements the help command.
 */
static void shell_command_help(void) {
  terminal_writestring("wigOSX commands:\n");
  terminal_writestring("  help     - show this help text\n");
  terminal_writestring("  clear    - clear the screen\n");
  terminal_writestring("  version  - show kernel version\n");
  terminal_writestring("  ticks    - show PIT tick count\n");
  terminal_writestring("  mem      - show detected memory summary\n");
  terminal_writestring("  pmm      - show physical memory manager summary\n");
  terminal_writestring(
      "  pmm_test - run a basic PMM allocation/free self-test\n");
  terminal_writestring("  about    - describe the current stage\n");
  terminal_writestring("  scroll   - print lines to test terminal scrolling\n");
}

/**
 * @brief Implements the version command.
 */
static void shell_command_version(void) {
  terminal_writestring("wigOSX ");
  terminal_writestring(WIGOSX_VERSION_STRING);
  terminal_putchar('\n');
}

/**
 * @brief Implements the ticks command.
 */
static void shell_command_ticks(void) {
  terminal_writestring("PIT ticks since initialization: ");
  shell_print_uint32(pit_get_ticks());
  terminal_putchar('\n');
}

/**
 * @brief Implements the about command.
 */
static void shell_command_about(void) {
  terminal_writestring(WIGOSX_STAGE_LABEL);
  terminal_writestring(".\n");
  terminal_writestring(
      "The kernel now tracks physical 4 KiB frames with a bitmap PMM.\n");
}

/**
 * @brief Prints enough lines to test terminal scrolling.
 */
static void shell_command_scrolltest(void) {
  for (uint32_t i = 0; i < 40; i++) {
    terminal_writestring("scroll test line ");
    shell_print_uint32(i);
    terminal_putchar('\n');
  }
}

/**
 * @brief Executes one completed command line.
 *
 * @param command Null-terminated command string.
 */
static void shell_execute_command(const char* command) {
  if (shell_command_is_empty(command)) {
    return;
  }

  if (shell_strings_equal(command, "help")) {
    shell_command_help();
  } else if (shell_strings_equal(command, "clear")) {
    terminal_clear();
  } else if (shell_strings_equal(command, "version")) {
    shell_command_version();
  } else if (shell_strings_equal(command, "ticks")) {
    shell_command_ticks();
  } else if (shell_strings_equal(command, "mem")) {
    memory_print_summary();
  } else if (shell_strings_equal(command, "pmm")) {
    pmm_print_summary();
  } else if (shell_strings_equal(command, "pmm_test")) {
    if (pmm_run_basic_self_test()) {
      terminal_writestring("PMM self-test passed.\n");
    } else {
      terminal_writestring("PMM self-test failed.\n");
    }
  } else if (shell_strings_equal(command, "about")) {
    shell_command_about();
  } else if (shell_strings_equal(command, "scroll")) {
    shell_command_scrolltest();
  } else {
    terminal_writestring("Unknown command: ");
    terminal_writestring(command);
    terminal_putchar('\n');
    terminal_writestring("Type 'help' for available commands.\n");
  }
}

/**
 * @brief Clears the current command buffer.
 */
static void shell_reset_buffer(void) {
  for (size_t i = 0; i < SHELL_BUFFER_SIZE; i++) {
    shell_buffer[i] = '\0';
  }

  shell_buffer_length = 0;
}

void shell_initialize(void) {
  shell_reset_buffer();

  terminal_writestring("Stage 8 shell initialized.\n");
  terminal_writestring("Type 'help' for commands.\n");
  shell_print_prompt();

  serial_writestring("[wigOSX] Stage 8: shell initialized.\n");
}

void shell_handle_character(char ascii) {
  if (ascii == '\n') {
    terminal_putchar('\n');

    shell_buffer[shell_buffer_length] = '\0';
    shell_execute_command(shell_buffer);
    shell_reset_buffer();
    shell_print_prompt();
    return;
  }

  if (ascii == '\b') {
    if (shell_buffer_length > 0) {
      shell_buffer_length--;
      shell_buffer[shell_buffer_length] = '\0';
      terminal_putchar('\b');
    }

    return;
  }

  if (ascii == '\t') {
    return;
  }

  if (shell_buffer_length >= SHELL_BUFFER_SIZE - 1) {
    serial_writestring("[wigOSX] Shell input buffer full.\n");
    return;
  }

  shell_buffer[shell_buffer_length] = ascii;
  shell_buffer_length++;

  terminal_putchar(ascii);
}
