#ifndef KERNEL_DRIVERS_KEYBOARD_H
#define KERNEL_DRIVERS_KEYBOARD_H

#include <stdbool.h>

/**
 * @file keyboard.h
 * @brief PS/2 keyboard driver interface.
 */

/**
 * Processes one PS/2 keyboard interrupt.
 *
 * @param out_ascii Receives a translated character when one is available.
 *
 * @return true when a translated character was produced.
 */
bool keyboard_handle_interrupt(char* out_ascii);

#endif /* KERNEL_DRIVERS_KEYBOARD_H */