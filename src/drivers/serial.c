#include <kernel/drivers/serial.h>

/**
 * @file serial.c
 * @brief Polling-based COM1 serial logging driver.
 *
 * COM1 is the first standard PC serial port.
 * Its base I/O port address is 0x3F8.
 */

/**
 * @brief Tracks whether COM1 initialization has completed.
 */
static bool serial_initialized = false;

/**
 * @brief Writes one byte to an x86 I/O port.
 *
 * @param port I/O port address.
 * @param value Byte to write.
 *
 * Write one byte to an x86 I/O port.
 *
 * Security note:
 * Raw port I/O is powerful and dangerous.
 * We keep it private inside this driver instead of exposing it to kernel.c.
 */
static inline void outb(uint16_t port, uint8_t value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * @brief Reads one byte from an x86 I/O port.
 *
 * @param port I/O port address.
 * @return Byte read from the port.
 */
static inline uint8_t inb(uint16_t port) {
  uint8_t value;
  __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}

/**
 * @brief Computes the length of a null-terminated string.
 *
 * @param str String to measure.
 * @return Number of bytes before the null terminator.
 */
static size_t serial_strlen(const char *str) {
  size_t length = 0;

  while (str[length] != '\0') {
    length++;
  }

  return length;
}

/**
 * @brief Reports whether COM1 is ready to accept another byte.
 *
 * @return true if the transmitter holding register is empty.
 *
 * The Line Status Register is at COM1 + 5.
 * Bit 5 tells us whether the transmitter is empty.
 */
static bool serial_can_transmit(void) {
  return (inb(SERIAL_PORT_COM1 + 5) & 0x20) != 0;
}

/**
 * @brief Initializes COM1 for polling-based serial output.
 *
 * Initialize COM1 for polling-based serial output.
 *
 * This sets:
 * - 38400 baud
 * - 8 data bits
 * - no parity
 * - 1 stop bit
 * - FIFO enabled
 */
void serial_initialize(void) {
  /*
   * Disable serial interrupts. This driver uses polling for now, which keeps
   * early boot logging independent from the IDT/PIC setup.
   */
  outb(SERIAL_PORT_COM1 + 1, 0x00);

  /* Enable DLAB so the low/high divisor bytes are accessible. */
  outb(SERIAL_PORT_COM1 + 3, 0x80);

  /* Divisor 3 gives 38400 baud from the standard 115200 Hz serial clock. */
  outb(SERIAL_PORT_COM1 + 0, 0x03);
  outb(SERIAL_PORT_COM1 + 1, 0x00);

  /* Use 8 data bits, no parity, one stop bit, and disable DLAB again. */
  outb(SERIAL_PORT_COM1 + 3, 0x03);

  /* Enable FIFO, clear it, and use a 14-byte threshold. */
  outb(SERIAL_PORT_COM1 + 2, 0xC7);

  /*
   * Enable IRQ output lines and mark RTS/DSR ready. IER remains disabled, so
   * output is still polling-based.
   */
  outb(SERIAL_PORT_COM1 + 4, 0x0B);

  serial_initialized = true;
}

bool serial_is_initialized(void) {
  return serial_initialized;
}

void serial_putchar(char c) {
  if (!serial_initialized) {
    return;
  }

  /* Many serial terminals expect CRLF, so '\n' is expanded to "\r\n". */
  if (c == '\n') {
    serial_putchar('\r');
  }

  while (!serial_can_transmit()) {
    /*
     * Busy waiting is acceptable during early boot. Interrupt-driven serial
     * output can replace this once the kernel has a scheduler or wait queues.
     */
  }

  outb(SERIAL_PORT_COM1, (uint8_t)c);
}

void serial_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    serial_putchar(data[i]);
  }
}

void serial_writestring(const char *data) {
  serial_write(data, serial_strlen(data));
}
