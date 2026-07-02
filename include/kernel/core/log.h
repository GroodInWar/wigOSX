#ifndef KERNEL_CORE_LOG_H
#define KERNEL_CORE_LOG_H

#include <stdint.h>

/**
 * @file log.h
 * @brief Shared kernel logging helpers.
 *
 * The logging layer is the only generic kernel layer that should know how to
 * write to both the VGA terminal and serial output.
 */

void klog_putchar(char c);
void klog_writestring(const char* text);
void klog_write_uint32(uint32_t value);
void klog_write_hex32(uint32_t value);

#endif /* KERNEL_CORE_LOG_H */