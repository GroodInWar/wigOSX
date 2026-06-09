#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SERIAL_PORT_COM1 0x3F8

void serial_initialize(void);
bool serial_is_initialized(void);

void serial_putchar(char c);
void serial_write(const char *data, size_t size);
void serial_writestring(const char *data);

#endif /* KERNEL_SERIAL_H */