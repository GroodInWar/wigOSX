#include <kernel/core/time.h>
#include <kernel/drivers/pit.h>

/**
 * @file time.c
 * @brief Generic early-kernel time facade.
 */

/**
 * @brief Returns the current PIT tick count.
 */
uint32_t ktime_get_ticks(void) { return pit_get_ticks(); }
