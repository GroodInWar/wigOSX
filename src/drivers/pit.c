#include <kernel/arch/i386/io.h>
#include <kernel/drivers/pit.h>
#include <kernel/drivers/serial.h>
#include <stdint.h>

/**
 * @file pit.c
 * @brief Programmable Interval Timer driver.
 *
 * The PIT drives IRQ0 and provides a simple periodic timer source before more
 * modern timers such as APIC or HPET exist in the kernel.
 */

/** @brief PIT input clock frequency in hertz. */
#define PIT_BASE_FREQUENCY 1193182
/** @brief Data port for PIT channel 0, the channel wired to IRQ0. */
#define PIT_CHANNEL0_PORT 0x40
/** @brief PIT mode/command register port. */
#define PIT_COMMAND_PORT 0x43

/** @brief Command bits selecting channel 0. */
#define PIT_COMMAND_CHANNEL0 0x00
/** @brief Command bits requesting low-byte then high-byte divisor writes. */
#define PIT_COMMAND_ACCESS_LOHI 0x30
/** @brief Command bits selecting square-wave generator mode. */
#define PIT_COMMAND_MODE3 0x06
/** @brief Command bit selecting binary counter mode. */
#define PIT_COMMAND_BINARY 0x00

/**
 * @brief Number of PIT interrupts handled since the last initialization.
 */
static uint32_t pit_ticks = 0;

/**
 * @brief Programs PIT channel 0 to generate periodic IRQ0 interrupts.
 *
 * @param frequency_hz Desired interrupt frequency in hertz.
 */
void pit_initialize(uint32_t frequency_hz) {
  uint32_t divisor = PIT_BASE_FREQUENCY / frequency_hz;

  outb(PIT_COMMAND_PORT, PIT_COMMAND_CHANNEL0 | PIT_COMMAND_ACCESS_LOHI |
                             PIT_COMMAND_MODE3 | PIT_COMMAND_BINARY);

  outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
  outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);

  pit_ticks = 0;
}

/**
 * @brief Handles one PIT interrupt and updates timer bookkeeping.
 *
 * The serial output is a temporary heartbeat while the kernel has no general
 * integer formatting or scheduler-visible time API.
 */
void pit_handle_interrupt(void) {
  pit_ticks++;

  if ((pit_ticks % 100) == 0) {
    serial_writestring("[wigOSX] PIT tick: ");

    /*
     * Very small temporary tick output.
     * TODO: Later I should add a real integer printing helper.
     */
    if (pit_ticks == 100) {
      serial_writestring("100\n");
    } else if (pit_ticks == 200) {
      serial_writestring("200\n");
    } else if (pit_ticks == 300) {
      serial_writestring("300\n");
    } else {
      serial_writestring("another second passed\n");
    }
  }
}

/**
 * @brief Returns the number of timer interrupts handled since initialization.
 */
uint32_t pit_get_ticks(void) { return pit_ticks; }
