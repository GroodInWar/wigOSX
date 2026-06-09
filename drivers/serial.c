#include <kernel/serial.h>

/*
 * COM1 is the first standard PC serial port.
 * Its base I/O port address is 0x3F8.
 */

static bool serial_initialized = false;

/*
 * Write one byte to an x86 I/O port.
 *
 * Security note:
 * Raw port I/O is powerful and dangerous.
 * We keep it private inside this driver instead of exposing it to kernel.c.
 */
static inline void outb(uint16_t port, uint8_t value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * Read one byte from an x86 I/O port.
 */
static inline uint8_t inb(uint16_t port) {
  uint8_t value;
  __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}

static size_t serial_strlen(const char *str) {
  size_t length = 0;

  while (str[length] != '\0') {
    length++;
  }

  return length;
}

/*
 * The Line Status Register is at COM1 + 5.
 * Bit 5 tells us whether the transmitter is empty.
 */
static bool serial_can_transmit(void) {
  return (inb(SERIAL_PORT_COM1 + 5) & 0x20) != 0;
}

/*
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
   * Disable serial interrupts.
   * We are using polling for now, not interrupt-driven serial.
   */
  outb(SERIAL_PORT_COM1 + 1, 0x00);

  /*
   * Enable DLAB so we can set the baud rate divisor.
   */
  outb(SERIAL_PORT_COM1 + 3, 0x80);

  /*
   * Divisor 3 gives 38400 baud.
   */
  outb(SERIAL_PORT_COM1 + 0, 0x03);
  outb(SERIAL_PORT_COM1 + 1, 0x00);

  /*
   * 8 bits, no parity, one stop bit.
   * This also disables DLAB again.
   */
  outb(SERIAL_PORT_COM1 + 3, 0x03);

  /*
   * Enable FIFO, clear it, and use a 14-byte threshold.
   */
  outb(SERIAL_PORT_COM1 + 2, 0xC7);

  /*
   * Enable IRQ output lines and mark RTS/DSR ready.
   * We still are not using interrupts yet because IER is disabled above.
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

  /*
   * Many serial terminals expect CRLF for a newline.
   * So '\n' becomes '\r' followed by '\n'.
   */
  if (c == '\n') {
    serial_putchar('\r');
  }

  while (!serial_can_transmit()) {
    /*
     * Busy wait.
     * This is okay for Stage 3.
     * Later, interrupts can make this more efficient.
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