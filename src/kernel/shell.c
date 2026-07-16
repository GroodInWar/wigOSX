#include <kernel/core/console.h>
#include <kernel/core/format.h>
#include <kernel/core/log.h>
#include <kernel/core/memory.h>
#include <kernel/core/shell.h>
#include <kernel/core/time.h>
#include <kernel/core/version.h>
#include <kernel/mm/pmm.h>
#include <kernel/mm/vmm.h>
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
static void shell_print_prompt(void) { console_writestring(SHELL_PROMPT); }

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
  char buffer[11];

  if (kformat_uint32_decimal(value, buffer, sizeof(buffer))) {
    console_writestring(buffer);
  }
}

/**
 * @brief Implements the help command.
 */
static void shell_command_help(void) {
  console_writestring("wigOSX commands:\n");
  console_writestring("  help     - show this help text\n");
  console_writestring("  clear    - clear the screen\n");
  console_writestring("  version  - show kernel version\n");
  console_writestring("  ticks    - show PIT tick count\n");
  console_writestring("  mem      - show detected memory summary\n");
  console_writestring("  pmm      - show physical memory manager summary\n");
  console_writestring(
      "  pmm_test - run a basic PMM allocation/free self-test\n");
  console_writestring("  paging   - show paging and identity-map summary\n");
  console_writestring("  about    - describe the current stage\n");
  console_writestring("  scroll   - print lines to test terminal scrolling\n");
}

/**
 * @brief Implements the version command.
 */
static void shell_command_version(void) {
  console_writestring("wigOSX ");
  console_writestring(WIGOSX_VERSION_STRING);
  console_putchar('\n');
}

/**
 * @brief Implements the ticks command.
 */
static void shell_command_ticks(void) {
  console_writestring("PIT ticks since initialization: ");
  shell_print_uint32(ktime_get_ticks());
  console_putchar('\n');
}

/**
 * @brief Implements the about command.
 */
static void shell_command_about(void) {
  console_writestring(WIGOSX_STAGE_LABEL);
  console_writestring(".\n");
  console_writestring(
      "Kernel services now use explicit console, logging, panic, input, "
      "and interrupt boundaries.\n");
  console_writestring("Identity paging remains active for the first 16 MiB.\n");
}

/**
 * @brief Prints enough lines to test terminal scrolling.
 */
static void shell_command_scrolltest(void) {
  for (uint32_t i = 0; i < 40; i++) {
    console_writestring("scroll test line ");
    shell_print_uint32(i);
    console_putchar('\n');
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
    console_clear();
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
      console_writestring("PMM self-test passed.\n");
    } else {
      console_writestring("PMM self-test failed.\n");
    }
  } else if (shell_strings_equal(command, "paging")) {
    vmm_print_summary();
  } else if (shell_strings_equal(command, "about")) {
    shell_command_about();
  } else if (shell_strings_equal(command, "scroll")) {
    shell_command_scrolltest();
  } else {
    console_writestring("Unknown command: ");
    console_writestring(command);
    console_putchar('\n');
    console_writestring("Type 'help' for available commands.\n");
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

/**
 * @brief Initializes shell input state and prints the first prompt.
 */
void shell_initialize(void) {
  shell_reset_buffer();

  console_writestring(WIGOSX_STAGE_LABEL);
  console_writestring(" shell ready.\n");
  console_writestring("Type 'help' for commands.\n");
  shell_print_prompt();
}

/**
 * @brief Handles one translated keyboard character as shell input.
 */
void shell_handle_character(char ascii) {
  if (ascii == '\n') {
    console_putchar('\n');

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
      console_putchar('\b');
    }

    return;
  }

  if (ascii == '\t') {
    return;
  }

  if (shell_buffer_length >= SHELL_BUFFER_SIZE - 1) {
    klog_writestring("\n[WARNING] Shell input buffer full.\n");
    shell_print_prompt();
    return;
  }

  shell_buffer[shell_buffer_length] = ascii;
  shell_buffer_length++;

  console_putchar(ascii);
}
