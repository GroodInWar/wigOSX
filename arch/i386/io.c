#include <kernel/io.h>

/**
 * @file io.c
 * @brief i386 port I/O implementation.
 *
 * These helpers wrap privileged x86 IN/OUT instructions so device drivers can
 * talk to legacy hardware ports without duplicating inline assembly.
 */

/**
 * @brief Writes one byte to an x86 I/O port.
 *
 * The "Nd" constraint lets the compiler use an immediate port number when
 * possible, or DX for ports that cannot be encoded directly.
 */
void outb(uint16_t port, uint8_t value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * @brief Reads one byte from an x86 I/O port.
 *
 * @return Byte returned by the device at the requested port.
 */
uint8_t inb(uint16_t port) {
  uint8_t value;
  __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}

/**
 * @brief Waits briefly for slow legacy devices to observe previous I/O.
 *
 * Port 0x80 is historically used as a harmless delay target on PC-compatible
 * systems.
 */
void io_wait(void) { outb(0x80, 0); }
