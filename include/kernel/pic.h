#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H

#include <stdint.h>

/**
 * @file pic.h
 * @brief Public interface for the 8259 Programmable Interrupt Controller.
 */

/**
 * @brief IDT vector where remapped master PIC IRQs begin.
 */
#define PIC_MASTER_OFFSET 0x20

/**
 * @brief IDT vector where remapped slave PIC IRQs begin.
 */
#define PIC_SLAVE_OFFSET 0x28

/**
 * @brief Remaps the PIC away from CPU exception vectors.
 *
 * By default, old PC PIC IRQs overlap with CPU exception vectors.
 * This function moves IRQ0-IRQ15 to IDT vectors 32-47.
 */
void pic_remap(void);

/**
 * @brief Masks all hardware IRQ lines.
 */
void pic_mask_all(void);

/**
 * @brief Unmasks one IRQ line.
 *
 * @param irq IRQ number from 0 to 15.
 */
void pic_unmask_irq(uint8_t irq);

/**
 * @brief Sends End Of Interrupt to the PIC.
 *
 * @param irq IRQ number that was handled.
 */
void pic_send_eoi(uint8_t irq);

#endif /* KERNEL_PIC_H */