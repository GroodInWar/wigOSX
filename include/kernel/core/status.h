#ifndef KERNEL_CORE_STATUS_H
#define KERNEL_CORE_STATUS_H

#include <stdbool.h>

/**
 * @file status.h
 * @brief Shared status-code type for kernel subsystem initialization.
 */

/**
 * @brief Common kernel status codes used instead of hosted errno values.
 */
typedef enum {
  KERNEL_STATUS_OK = 0,
  KERNEL_STATUS_INVALID_ARGUMENT,
  KERNEL_STATUS_NOT_READY,
  KERNEL_STATUS_UNAVAILABLE,
  KERNEL_STATUS_MALFORMED_DATA,
  KERNEL_STATUS_OUT_OF_MEMORY,
  KERNEL_STATUS_HARDWARE_FAILURE,
  KERNEL_STATUS_ALREADY_INITIALIZED
} kernel_status_t;

/**
 * @brief Reports whether a kernel status represents success.
 *
 * @param status Status code to test.
 * @return true when @p status is KERNEL_STATUS_OK.
 */
static inline bool kernel_status_is_ok(kernel_status_t status) {
  return status == KERNEL_STATUS_OK;
}

#endif /* KERNEL_CORE_STATUS_H */
