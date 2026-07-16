#ifndef KERNEL_CORE_INTERRUPTS_H
#define KERNEL_CORE_INTERRUPTS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file interrupts.h
 * @brief Generic IRQ registration and dispatch interface.
 */

/**
 * @brief Number of legacy PIC IRQ lines supported by the dispatcher.
 */
#define IRQ_HANDLER_COUNT 16U

/**
 * @brief Signature for a registered hardware IRQ handler.
 */
typedef void (*irq_handler_t)(void);

/**
 * @brief Clears the registered IRQ handler table.
 */
void interrupts_initialize(void);

/**
 * @brief Registers a handler for one hardware IRQ line.
 *
 * @param irq IRQ number from 0 to IRQ_HANDLER_COUNT - 1.
 * @param handler Function to invoke when the IRQ is dispatched.
 * @return true if the handler was registered, false if the IRQ was invalid,
 * handler was NULL, or a handler was already present.
 */
bool interrupts_register_irq(uint8_t irq, irq_handler_t handler);

/**
 * @brief Dispatches one hardware IRQ to its registered handler.
 *
 * @param irq IRQ number from 0 to IRQ_HANDLER_COUNT - 1.
 */
void interrupts_dispatch_irq(uint8_t irq);

#endif /* KERNEL_CORE_INTERRUPTS_H */
