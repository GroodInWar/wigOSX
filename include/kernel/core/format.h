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

/**
 * @brief Formats a 32-bit unsigned integer as decimal text.
 *
 * @param value Value to format.
 * @param buffer Destination buffer.
 * @param buffer_size Size of @p buffer in bytes.
 * @return true if the formatted string fit in @p buffer.
 */
bool kformat_uint32_decimal(uint32_t value, char* buffer, size_t buffer_size);

/**
 * @brief Formats a 64-bit unsigned integer as decimal text.
 *
 * @param value Value to format.
 * @param buffer Destination buffer.
 * @param buffer_size Size of @p buffer in bytes.
 * @return true if the formatted string fit in @p buffer.
 */
bool kformat_uint64_decimal(uint64_t value, char* buffer, size_t buffer_size);

/**
 * @brief Formats a 32-bit unsigned integer as fixed-width hexadecimal text.
 *
 * @param value Value to format.
 * @param buffer Destination buffer.
 * @param buffer_size Size of @p buffer in bytes.
 * @return true if @p buffer can hold the "0x" prefix, eight hex digits, and
 * null terminator.
 */
bool kformat_uint32_hex(uint32_t value, char* buffer, size_t buffer_size);

#endif /* KERNEL_CORE_FORMAT_H */
