#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <stdint.h>

/**
 * @file io.h
 * @brief Public interface for low-level x86 port I/O.
 *
 */

/**
 * @brief Writes one byte to an x86 I/O port.
 *
 * @param port I/O port number.
 * @param value Byte value to write.
 */
void outb(uint16_t port, uint8_t value);

/**
 * @brief Reads one byte from an x86 I/O port.
 *
 * @param port I/O port number.
 * @return Byte read from the port.
 */
uint8_t inb(uint16_t port);

/**
 * @brief Small delay used after some legacy PIC/PIT I/O operations.
 */
void io_wait(void);

#endif /* KERNEL_IO_H */