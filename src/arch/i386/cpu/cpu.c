#include <kernel/arch/i386/cpu.h>

/**
 * @file cpu.c
 * @brief i386 CPU-control helper implementation.
 */

/**
 * @brief Enables maskable hardware interrupts with STI.
 */
void cpu_enable_interrupts(void) { __asm__ volatile("sti"); }

/**
 * @brief Disables maskable hardware interrupts with CLI.
 */
void cpu_disable_interrupts(void) { __asm__ volatile("cli"); }

/**
 * @brief Halts the CPU until an interrupt or reset occurs.
 */
void cpu_halt(void) { __asm__ volatile("hlt"); }
