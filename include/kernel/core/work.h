#ifndef KERNEL_CORE_WORK_H
#define KERNEL_CORE_WORK_H

#include <stdbool.h>

/**
 * @file work.h
 * @brief Foreground kernel work processing outside interrupt context.
 *
 */

/**
 * @brief Processes at most one unit of pending foreground kernel work.
 *
 * Work is always executed outside interrupt context. If no work is available,
 * the processor enters an interrupt-safe idle state.
 */
void kernel_work_run_once(void);

#endif /* KERNEL_CORE_WORK_H */