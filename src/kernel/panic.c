#include <kernel/arch/i386/cpu.h>
#include <kernel/core/log.h>
#include <kernel/core/panic.h>
#include <stddef.h>

/**
 * @file panic.c
 * @brief Fatal kernel error reporting and halt path.
 */

/**
 * @brief Prints a panic message, disables interrupts, and halts forever.
 */
void kernel_panic(const char* message) {
  cpu_disable_interrupts();

  klog_writestring("\n[KERNEL PANIC] ");

  if (message != NULL) {
    klog_writestring(message);
  } else {
    klog_writestring("Unspecified fatal error");
  }

  klog_putchar('\n');
  klog_writestring("System halted.\n");

  while (1) {
    cpu_halt();
  }
}
