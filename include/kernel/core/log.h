#ifndef KERNEL_CORE_LOG_H
#define KERNEL_CORE_LOG_H

#include <kernel/core/status.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @file log.h
 * @brief Shared diagnostic-output service.
 *
 * The logging layer is the only generic kernel layer that should know how to
 * write to both the VGA terminal and serial output.
 */

/**
 * @brief Initializes kernel diagnostic output.
 *
 * @return KERNEL_STATUS_OK when serial output is available, otherwise
 * KERNEL_STATUS_UNAVAILABLE while VGA console output remains usable.
 */
kernel_status_t klog_initialize(void);

/**
 * @brief Reports whether diagnostic output is mirrored to serial.
 *
 * @return true if serial logging initialized successfully.
 */
bool klog_has_serial_output(void);

/**
 * @brief Writes one diagnostic character to all active log sinks.
 *
 * @param character Character to write.
 */
void klog_putchar(char character);

/**
 * @brief Writes a null-terminated diagnostic string to all active log sinks.
 *
 * @param text String to write. NULL is ignored.
 */
void klog_writestring(const char* text);

/**
 * @brief Writes a 32-bit unsigned integer in decimal.
 *
 * @param value Value to write.
 */
void klog_write_uint32(uint32_t value);

/**
 * @brief Writes a 64-bit unsigned integer in decimal.
 *
 * @param value Value to write.
 */
void klog_write_uint64(uint64_t value);

/**
 * @brief Writes a 32-bit unsigned integer in fixed-width hexadecimal.
 *
 * @param value Value to write.
 */
void klog_write_hex32(uint32_t value);

#endif /* KERNEL_CORE_LOG_H */
