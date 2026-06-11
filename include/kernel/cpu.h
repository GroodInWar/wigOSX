#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

/**
 * @file cpu.h
 * @brief Small CPU-control helpers for i386 kernel code.
 */

/**
 * @brief Enables maskable hardware interrupts.
 */
static inline void cpu_enable_interrupts(void) {
  __asm__ volatile("sti");
}

/**
 * @brief Disables maskable hardware interrupts.
 */
static inline void cpu_disable_interrupts(void) {
  __asm__ volatile("cli");
}

/**
 * @brief Halts the CPU until the next interrupt.
 */
static inline void cpu_halt(void) {
  __asm__ volatile("hlt");
}

#endif /* KERNEL_CPU_H */