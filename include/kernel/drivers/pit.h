#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file pit.h
 * @brief Public interface for the Programmable Interval Timer.
 */

/**
 * @brief Initializes the PIT to generate periodic IRQ0 interrupts.
 *
 * @param frequency_hz Desired timer frequency in hertz.
 * @return true if the PIT was programmed successfully, false if the requested
 * frequency is outside the supported range.
 */
bool pit_initialize(uint32_t frequency_hz);

/**
 * @brief Handles one PIT timer interrupt.
 */
void pit_handle_interrupt(void);

/**
 * @brief Returns the number of PIT ticks since initialization.
 *
 * @return Tick count.
 */
uint32_t pit_get_ticks(void);

#endif /* KERNEL_PIT_H */