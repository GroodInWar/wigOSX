#include <kernel/core/interrupts.h>
#include <stddef.h>

/**
 * @file interrupts.c
 * @brief Generic IRQ handler registry used by architecture dispatch code.
 */

/**
 * @brief Registered handlers indexed by IRQ number.
 */
static irq_handler_t irq_handlers[IRQ_HANDLER_COUNT];

/**
 * @brief Clears the IRQ handler table.
 */
void interrupts_initialize(void) {
  for (uint8_t irq = 0; irq < IRQ_HANDLER_COUNT; irq++) {
    irq_handlers[irq] = NULL;
  }
}

/**
 * @brief Registers one handler for a hardware IRQ line.
 */
bool interrupts_register_irq(uint8_t irq, irq_handler_t handler) {
  if (irq >= IRQ_HANDLER_COUNT || handler == NULL) {
    return false;
  }

  if (irq_handlers[irq] != NULL) {
    return false;
  }

  irq_handlers[irq] = handler;
  return true;
}

/**
 * @brief Dispatches one IRQ to its registered handler when present.
 */
void interrupts_dispatch_irq(uint8_t irq) {
  if (irq >= IRQ_HANDLER_COUNT) {
    return;
  }

  irq_handler_t handler = irq_handlers[irq];

  if (handler != NULL) {
    handler();
  }
}
