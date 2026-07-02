#include <kernel/core/format.h>
#include <kernel/core/log.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <stdint.h>

void klog_putchar(char c) {
  terminal_putchar(c);

  if (serial_is_initialized()) {
    serial_putchar(c);
  }
}

void klog_writestring(const char* text) {
  if (text == 0) {
    return;
  }

  terminal_writestring(text);

  if (serial_is_initialized()) {
    serial_writestring(text);
  }
}

void klog_write_uint32(uint32_t value) {
  char buffer[11];

  if (kformat_uint32_decimal(value, buffer, sizeof(buffer))) {
    klog_writestring(buffer);
  }
}

void klog_write_hex32(uint32_t value) {
  char buffer[11];

  if (kformat_uint32_hex(value, buffer, sizeof(buffer))) {
    klog_writestring(buffer);
  }
}