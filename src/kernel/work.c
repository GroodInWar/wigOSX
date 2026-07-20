#include <kernel/arch/i386/cpu.h>
#include <kernel/core/input.h>
#include <kernel/core/shell.h>
#include <kernel/core/work.h>

/**
 * @file work.c
 * @brief Foreground processing for work deferred out of interrupt context.
 */

void kernel_work_run_once(void) {
  char ascii = '\0';

  /*
   * Prevent the IRQ producer from changing queue state while foreground code
   * checks for pending input.
   */
  cpu_disable_interrupts();

  if (input_try_read_character(&ascii)) {
    /*
     * Higher-level consumers must run with interrupts enabled and outside IRQ
     * context.
     */
    cpu_enable_interrupts();

    shell_handle_character(ascii);
    return;
  }

  /*
   * No work is pending while interrupts are disabled.
   *
   * STI followed immediately by HLT prevents the lost-wakeup race where an
   * interrupt arrives between the empty-work check and entering the idle state.
   */
  cpu_enable_interrupts_and_halt();
}