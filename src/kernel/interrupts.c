#include <kernel/core/interrupts.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file interrupts.c
 * @brief Generic IRQ registry, context tracking, and bounded diagnostics.
 */

/**
 * @brief Registered handlers indexed by IRQ number.
 */
static irq_handler_t irq_handlers[IRQ_HANDLER_COUNT];

/**
 * @brief Number of IRQ dispatches observed for each legacy PIC line.
 */
static volatile uint32_t irq_dispatch_counts[IRQ_HANDLER_COUNT];

/**
 * @brief Number of valid IRQs received without a registered handler.
 */
static volatile uint32_t unhandled_irq_count;

/**
 * @brief Number of invalid IRQ values passed to the dispatcher.
 */
static volatile uint32_t invalid_irq_count;

/**
 * @brief Current hardware interrupt-context nesting depth.
 */
static volatile uint32_t interrupt_context_depth;

/**
 * @brief Increments a diagnostic counter without wrapping to zero.
 */
static void interrupts_increment_saturating(volatile uint32_t* counter) {
  if (*counter != UINT32_MAX) {
    (*counter)++;
  }
}

/**
 * @brief Initializes the IRQ registry and all diagnostic state.
 */
void interrupts_initialize(void) {
  for (uint8_t irq = 0; irq < IRQ_HANDLER_COUNT; irq++) {
    irq_handlers[irq] = NULL;
    irq_dispatch_counts[irq] = 0;
  }

  unhandled_irq_count = 0;
  invalid_irq_count = 0;
  interrupt_context_depth = 0;
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
 * @brief Records entry into hardware interrupt context.
 */
bool interrupts_context_enter(void) {
  if (interrupt_context_depth == UINT32_MAX) {
    return false;
  }

  interrupt_context_depth++;
  return true;
}

/**
 * @brief Records exit from hardware interrupt context.
 */
bool interrupts_context_exit(void) {
  if (interrupt_context_depth == 0) {
    return false;
  }

  interrupt_context_depth--;
  return true;
}

/**
 * @brief Returns whether execution is inside hardware interrupt context.
 */
bool interrupts_is_in_context(void) { return interrupt_context_depth != 0; }

/**
 * @brief Returns the current interrupt-context nesting depth.
 */
uint32_t interrupts_get_context_depth(void) { return interrupt_context_depth; }

/**
 * @brief Dispatches one IRQ and records bounded diagnostic counters.
 */
void interrupts_dispatch_irq(uint8_t irq) {
  if (irq >= IRQ_HANDLER_COUNT) {
    interrupts_increment_saturating(&invalid_irq_count);
    return;
  }

  interrupts_increment_saturating(&irq_dispatch_counts[irq]);

  irq_handler_t handler = irq_handlers[irq];

  if (handler == NULL) {
    interrupts_increment_saturating(&unhandled_irq_count);
    return;
  }

  handler();
}

/**
 * @brief Returns the dispatch count for one IRQ line.
 */
uint32_t interrupts_get_irq_count(uint8_t irq) {
  if (irq >= IRQ_HANDLER_COUNT) {
    return 0;
  }

  return irq_dispatch_counts[irq];
}

/**
 * @brief Returns the total number of unhandled valid IRQs.
 */
uint32_t interrupts_get_unhandled_irq_count(void) {
  return unhandled_irq_count;
}

/**
 * @brief Returns the total number of invalid IRQ dispatch requests.
 */
uint32_t interrupts_get_invalid_irq_count(void) { return invalid_irq_count; }
