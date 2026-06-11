#ifndef KERNEL_DRIVERS_KEYBOARD_H
#define KERNEL_DRIVERS_KEYBOARD_H

/**
 * @file keyboard.h
 * @brief Public interface for the PS/2 keyboard driver.
 */

/**
 * @brief Handles one PS/2 keyboard interrupt.
 *
 * This function is called from the generic IRQ handler when IRQ1 fires. It
 * reads the keyboard scancode from the PS/2 data port and processes the key
 * event.
 */
void keyboard_handle_interrupt(void);

#endif /* KERNEL_DRIVERS_KEYBOARD_H */