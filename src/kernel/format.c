#include <kernel/core/format.h>

bool kformat_uint32_decimal(uint32_t value, char* buffer, size_t buffer_size) {
  char digits[10];
  size_t digit_count = 0;
  size_t output_index = 0;

  if (buffer == 0 || buffer_size == 0) {
    return false;
  }

  if (value == 0) {
    if (buffer_size < 2) {
      return false;
    }

    buffer[0] = '0';
    buffer[1] = '\0';
    return true;
  }

  while (value > 0) {
    digits[digit_count] = (char)('0' + (value % 10));
    value = value / 10;
    digit_count++;
  }

  if (buffer_size <= digit_count) {
    return false;
  }

  while (digit_count > 0) {
    digit_count--;
    buffer[output_index] = digits[digit_count];
    output_index++;
  }

  buffer[output_index] = '\0';
  return true;
}

bool kformat_uint32_hex(uint32_t value, char* buffer, size_t buffer_size) {
  const char hex_digits[] = "0123456789ABCDEF";

  if (buffer == 0 || buffer_size < 11) {
    return false;
  }

  buffer[0] = '0';
  buffer[1] = 'x';

  for (uint32_t i = 0; i < 8; i++) {
    uint32_t shift = 28U - (i * 4U);
    uint32_t digit = (value >> shift) & 0xFU;

    buffer[2 + i] = hex_digits[digit];
  }

  buffer[10] = '\0';
  return true;
}