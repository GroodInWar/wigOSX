#include <kernel/core/console.h>
#include <kernel/core/format.h>
#include <kernel/core/log.h>
#include <kernel/drivers/serial.h>
#include <stddef.h>

/**
 * @file log.c
 * @brief Diagnostic output fan-out for console and serial sinks.
 */

/**
 * @brief Tracks whether serial output is active for log mirroring.
 */
static bool klog_serial_available = false;

/**
 * @brief Initializes serial logging and leaves console logging available.
 */
kernel_status_t klog_initialize(void) {
  klog_serial_available = serial_initialize();

  if (!klog_serial_available) {
    return KERNEL_STATUS_UNAVAILABLE;
  }

  return KERNEL_STATUS_OK;
}

/**
 * @brief Reports whether diagnostic output is mirrored to serial.
 */
bool klog_has_serial_output(void) { return klog_serial_available; }

/**
 * @brief Writes one character to the console and, when available, serial.
 */
void klog_putchar(char character) {
  console_putchar(character);

  if (klog_serial_available) {
    serial_putchar(character);
  }
}

/**
 * @brief Writes one string to the console and, when available, serial.
 */
void klog_writestring(const char* text) {
  if (text == NULL) {
    return;
  }

  console_writestring(text);

  if (klog_serial_available) {
    serial_writestring(text);
  }
}

/**
 * @brief Formats and writes a 32-bit unsigned integer in decimal.
 */
void klog_write_uint32(uint32_t value) {
  char buffer[11];

  if (kformat_uint32_decimal(value, buffer, sizeof(buffer))) {
    klog_writestring(buffer);
  }
}

/**
 * @brief Formats and writes a 64-bit unsigned integer in decimal.
 */
void klog_write_uint64(uint64_t value) {
  char buffer[21];

  if (kformat_uint64_decimal(value, buffer, sizeof(buffer))) {
    klog_writestring(buffer);
  }
}

/**
 * @brief Formats and writes a 32-bit unsigned integer in hexadecimal.
 */
void klog_write_hex32(uint32_t value) {
  char buffer[11];

  if (kformat_uint32_hex(value, buffer, sizeof(buffer))) {
    klog_writestring(buffer);
  }
}
