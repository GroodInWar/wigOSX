#ifndef KERNEL_CORE_INPUT_H
#define KERNEL_CORE_INPUT_H

/**
 * @file input.h
 * @brief Adapter between keyboard input and the active kernel consumer.
 */

/**
 * @brief Handles one keyboard IRQ and forwards produced characters.
 */
void input_handle_keyboard_interrupt(void);

#endif /* KERNEL_CORE_INPUT_H */
