#ifndef KERNEL_CORE_CONSOLE_H
#define KERNEL_CORE_CONSOLE_H

#include <stdbool.h>

/**
 * @file console.h
 * @brief Interactive kernel console interface.
 */

/**
 * @brief Initializes console output through the active terminal backend.
 */
void console_initialize(void);

/**
 * @brief Reports whether the console has been initialized.
 *
 * @return true after console_initialize() has completed.
 */
bool console_is_initialized(void);

/**
 * @brief Writes one character to the console when initialized.
 *
 * @param character Character or supported terminal control to write.
 */
void console_putchar(char character);

/**
 * @brief Writes a null-terminated string to the console when initialized.
 *
 * @param text Null-terminated string to write. NULL is ignored.
 */
void console_writestring(const char* text);

/**
 * @brief Clears the console display when initialized.
 */
void console_clear(void);

#endif /* KERNEL_CORE_CONSOLE_H */
