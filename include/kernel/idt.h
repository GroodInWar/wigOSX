#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

/**
 * @file idt.h
 * @brief Public interface for the Interrupt Descriptor Table.
 */

/**
 * @brief Number of entries in the i386 IDT.
 */
#define IDT_ENTRY_COUNT 256

/**
 * @brief Number of CPU exception vectors reserved by the processor.
 */
#define IDT_CPU_EXCEPTION_COUNT 32

/**
 * @brief Kernel code segment selector from the GDT.
 */
#define IDT_KERNEL_CODE_SELECTOR 0x08

/**
 * @brief Present 32-bit interrupt gate usable from ring 0.
 */
#define IDT_INTERRUPT_GATE_RING0 0x8E

/**
 * @brief Initializes and loads the Interrupt Descriptor Table.
 */
void idt_initialize(void);

#endif /* KERNEL_IDT_H */