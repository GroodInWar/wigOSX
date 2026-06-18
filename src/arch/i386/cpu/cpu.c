#include <kernel/arch/i386/cpu.h>

/**
 * @file cpu.c
 * @brief i386 CPU-control helper implementation.
 */

void cpu_enable_interrupts(void) { __asm__ volatile("sti"); }

void cpu_disable_interrupts(void) { __asm__ volatile("cli"); }

void cpu_halt(void) { __asm__ volatile("hlt"); }
