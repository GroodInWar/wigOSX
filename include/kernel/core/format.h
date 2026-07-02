#ifndef KERNEL_CORE_FORMAT_H
#define KERNEL_CORE_FORMAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file format.h
 * @brief Small freestanding formatting helpers for kernel output.
 *
 * These helpers do not write to hardware. They only convert values into
 * caller-provided buffers so higher-level output code can decide where to send
 * the text.
 */

bool kformat_uint32_decimal(uint32_t value, char* buffer, size_t buffer_size);
bool kformat_uint32_hex(uint32_t value, char* buffer, size_t buffer_size);

#endif /* KERNEL_CORE_FORMAT_H */