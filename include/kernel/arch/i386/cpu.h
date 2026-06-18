#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

/**
 * @file cpu.h
 * @brief Small CPU-control helpers for i386 kernel code.
 */

/**
 * @brief Enables maskable hardware interrupts.
 */
void cpu_enable_interrupts(void);

/**
 * @brief Disables maskable hardware interrupts.
 */
void cpu_disable_interrupts(void);

/**
 * @brief Halts the CPU until the next interrupt.
 *
 * If interrupts are enabled, execution resumes after an interrupt is
 * delivered and handled.
 */
void cpu_halt(void);

#endif /* KERNEL_CPU_H */
