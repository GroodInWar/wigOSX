#ifndef KERNEL_CORE_TIME_H
#define KERNEL_CORE_TIME_H

#include <stdint.h>

/**
 * @file time.h
 * @brief Generic early-kernel time interface.
 */

/**
 * @brief Returns timer ticks observed by the kernel time source.
 *
 * @return Number of PIT ticks since timer initialization.
 */
uint32_t ktime_get_ticks(void);

#endif /* KERNEL_CORE_TIME_H */
