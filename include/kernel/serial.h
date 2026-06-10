#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file serial.h
 * @brief Public interface for polling-based COM1 serial logging.
 */

/**
 * @brief Base I/O port address for the first PC serial port.
 */
#define SERIAL_PORT_COM1 0x3F8

/**
 * @brief Initializes COM1 for serial output.
 */
void serial_initialize(void);

/**
 * @brief Reports whether the serial driver has been initialized.
 *
 * @return true if serial output is ready, false otherwise.
 */
bool serial_is_initialized(void);

/**
 * @brief Writes one character to COM1.
 *
 * @param c Character to write. Newlines are emitted as CRLF.
 */
void serial_putchar(char c);

/**
 * @brief Writes a fixed-size byte range to COM1.
 *
 * @param data Pointer to the bytes to write.
 * @param size Number of bytes to write.
 */
void serial_write(const char *data, size_t size);

/**
 * @brief Writes a null-terminated string to COM1.
 *
 * @param data Null-terminated string to write.
 */
void serial_writestring(const char *data);

#endif /* KERNEL_SERIAL_H */
