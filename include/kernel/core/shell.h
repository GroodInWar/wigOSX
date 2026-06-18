#ifndef KERNEL_CORE_SHELL_H
#define KERNEL_CORE_SHELL_H

/**
 * @file shell.h
 * @brief Public interface for the tiny Stage 8 kernel shell.
 */

/**
 * @brief Initializes the shell input state and prints the first prompt.
 */
void shell_initialize(void);

/**
 * @brief Gives one translated keyboard character to the shell.
 *
 * @param ascii ASCII character produced by the keyboard driver.
 */
void shell_handle_character(char ascii);

#endif /* KERNEL_CORE_SHELL_H */
