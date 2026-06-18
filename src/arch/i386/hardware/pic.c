#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/pic.h>
#include <stdint.h>

/**
 * @file pic.c
 * @brief i386 8259 PIC initialization and control.
 *
 * The legacy PC interrupt controller delivers IRQ0-IRQ15 to the CPU. During
 * early kernel boot we remap those interrupts away from CPU exception vectors
 * and mask everything except devices that have handlers.
 */

/** @brief Master PIC command port. */
#define PIC_MASTER_COMMAND 0x20
/** @brief Master PIC data and mask port. */
#define PIC_MASTER_DATA 0x21
/** @brief Slave PIC command port. */
#define PIC_SLAVE_COMMAND 0xA0
/** @brief Slave PIC data and mask port. */
#define PIC_SLAVE_DATA 0xA1

/** @brief End-of-interrupt command sent after handling an IRQ. */
#define PIC_COMMAND_EOI 0x20

/** @brief Initialization Command Word 1 bit that starts PIC initialization. */
#define PIC_ICW1_INIT 0x10
/** @brief Initialization Command Word 1 bit indicating ICW4 will follow. */
#define PIC_ICW1_ICW4 0x01
/** @brief Initialization Command Word 4 value for 8086/88 mode. */
#define PIC_ICW4_8086 0x01

/**
 * @brief Remaps IRQ0-IRQ15 to the configured IDT vector offsets.
 *
 * The BIOS default maps IRQs over CPU exception vectors, so protected-mode
 * kernels normally move the master PIC to 0x20 and the slave PIC to 0x28.
 * Existing masks are preserved across the initialization sequence.
 */
void pic_remap(void) {
  uint8_t master_mask = inb(PIC_MASTER_DATA);
  uint8_t slave_mask = inb(PIC_SLAVE_DATA);

  /* Start initialization and tell each PIC that ICW4 will be sent. */
  outb(PIC_MASTER_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
  io_wait();
  outb(PIC_SLAVE_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
  io_wait();

  /* Set the new vector bases for the master and slave PICs. */
  outb(PIC_MASTER_DATA, PIC_MASTER_OFFSET);
  io_wait();
  outb(PIC_SLAVE_DATA, PIC_SLAVE_OFFSET);
  io_wait();

  /* Describe the wiring: slave is connected to master's IRQ2 line. */
  outb(PIC_MASTER_DATA, 0x04);
  io_wait();
  outb(PIC_SLAVE_DATA, 0x02);
  io_wait();

  /* Put both controllers into 8086 mode. */
  outb(PIC_MASTER_DATA, PIC_ICW4_8086);
  io_wait();
  outb(PIC_SLAVE_DATA, PIC_ICW4_8086);
  io_wait();

  /* Restore the IRQ masks that were active before remapping. */
  outb(PIC_MASTER_DATA, master_mask);
  outb(PIC_SLAVE_DATA, slave_mask);
}

/**
 * @brief Disables delivery for every hardware IRQ line.
 */
void pic_mask_all(void) {
  outb(PIC_MASTER_DATA, 0xFF);
  outb(PIC_SLAVE_DATA, 0xFF);
}

bool pic_unmask_irq(uint8_t irq) {
  if (irq > 15) {
    return false;
  }

  if (irq < 8) {
    uint8_t value = inb(PIC_MASTER_DATA);
    value &= ~(1 << irq);
    outb(PIC_MASTER_DATA, value);
    return true;
  }

  uint8_t slave_irq = irq - 8;

  uint8_t slave_mask = inb(PIC_SLAVE_DATA);
  slave_mask &= ~(1 << slave_irq);
  outb(PIC_SLAVE_DATA, slave_mask);

  /*
   * The slave PIC is connected through IRQ2 on the master PIC.
   * If IRQ2 stays masked, IRQ8-IRQ15 cannot reach the CPU.
   */
  uint8_t master_mask = inb(PIC_MASTER_DATA);
  master_mask &= ~(1 << 2);
  outb(PIC_MASTER_DATA, master_mask);

  return true;
}

void pic_send_eoi(uint8_t irq) {
  if (irq > 15) {
    return;
  }

  if (irq >= 8) {
    outb(PIC_SLAVE_COMMAND, PIC_COMMAND_EOI);
  }

  outb(PIC_MASTER_COMMAND, PIC_COMMAND_EOI);
}
