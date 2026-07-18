#ifndef KERNEL_CORE_INPUT_H
#define KERNEL_CORE_INPUT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file input.h
 * @brief Queued input service between hardware IRQs and foreground consumers.
 */

/**
 * @brief Initializes the input character queue.
 *
 * Call this before keyboard IRQ1 is unmasked.
 */
void input_initialize(void);

/**
 * @brief Handles one keyboard IRQ and queues any translated character.
 *
 * This function must remain short and must not invoke the shell, perform
 * lengthy output, or execute foreground commands.
 */
void input_handle_keyboard_interrupt(void);

/**
 * @brief Removes one character from the input queue.
 *
 * @param out_ascii Destination for the dequeued character.
 * @return true when a character was returned, otherwise false.
 */
bool input_try_read_character(char* out_ascii);

/**
 * @brief Returns the approximate number of queued characters.
 *
 * @return Number of characters waiting for foreground processing.
 */
uint32_t input_get_pending_character_count(void);

/**
 * @brief Returns the number of characters discarded because the queue was full.
 *
 * @return Saturating count of dropped input characters.
 */
uint32_t input_get_dropped_character_count(void);

#endif /* KERNEL_CORE_INPUT_H */