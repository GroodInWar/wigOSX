#ifndef KERNEL_CORE_INTERRUPTS_H
#define KERNEL_CORE_INTERRUPTS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file interrupts.h
 * @brief Generic IRQ registration, dispatch, context, and diagnostics
 * interface.
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
 * @brief Initializes the IRQ registry, context state, and diagnostic counters.
 */
void interrupts_initialize(void);

/**
 * @brief Registers a handler for one hardware IRQ line.
 *
 * @param irq IRQ number from 0 to IRQ_HANDLER_COUNT - 1.
 * @param handler Function to invoke when the IRQ is dispatched.
 * @return true when registration succeeds, otherwise false.
 */
bool interrupts_register_irq(uint8_t irq, irq_handler_t handler);

/**
 * @brief Records entry into hardware interrupt context.
 *
 * The architecture-level IRQ dispatcher must call this before invoking the
 * generic IRQ dispatcher.
 *
 * @return true when the context depth was incremented, otherwise false.
 */
bool interrupts_context_enter(void);

/**
 * @brief Records exit from hardware interrupt context.
 *
 * The architecture-level IRQ dispatcher must call this after completing the
 * IRQ, including sending the controller's end-of-interrupt signal.
 *
 * @return true when the context depth was decremented, otherwise false.
 */
bool interrupts_context_exit(void);

/**
 * @brief Returns whether execution is currently inside hardware IRQ context.
 */
bool interrupts_is_in_context(void);

/**
 * @brief Returns the current hardware interrupt-context depth.
 */
uint32_t interrupts_get_context_depth(void);

/**
 * @brief Dispatches one hardware IRQ to its registered handler.
 *
 * Invalid and unregistered IRQs are counted but not logged from interrupt
 * context.
 *
 * @param irq IRQ number from 0 to IRQ_HANDLER_COUNT - 1.
 */
void interrupts_dispatch_irq(uint8_t irq);

/**
 * @brief Returns the number of times an IRQ line was dispatched.
 *
 * @param irq IRQ number to inspect.
 * @return Dispatch count, or zero when irq is invalid.
 */
uint32_t interrupts_get_irq_count(uint8_t irq);

/**
 * @brief Returns the number of valid IRQs without registered handlers.
 */
uint32_t interrupts_get_unhandled_irq_count(void);

/**
 * @brief Returns the number of invalid IRQ dispatch requests.
 */
uint32_t interrupts_get_invalid_irq_count(void);

#endif /* KERNEL_CORE_INTERRUPTS_H */
