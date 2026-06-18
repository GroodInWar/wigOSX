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

/**
 * @brief Saved CPU state passed to the common interrupt handlers.
 *
 * The structure layout is private to the IDT implementation. Public
 * declarations only need this incomplete type because handlers receive it by
 * pointer.
 */
struct interrupt_frame;

/**
 * @brief Handles a CPU exception dispatched by an assembly ISR stub.
 *
 * @param frame Saved CPU state for the interrupted context.
 */
void isr_handler(struct interrupt_frame* frame);

/**
 * @brief Handles a hardware interrupt dispatched by an assembly IRQ stub.
 *
 * @param frame Saved CPU state for the interrupted context.
 */
void irq_handler(struct interrupt_frame* frame);

#endif /* KERNEL_IDT_H */
