.section .text

/**
 * @file isr_stubs.s
 * @brief i386 CPU exception and hardware interrupt entry stubs.
 *
 * These stubs convert CPU-specific interrupt entry into a consistent stack
 * layout for the C handlers in src/arch/i386/interrupts/idt.c.
 */

.extern isr_handler
.extern irq_handler

/**
 * @brief Defines a CPU exception stub for vectors without CPU error codes.
 *
 * A synthetic zero error code is pushed so every exception reaches the common
 * handler with the same frame shape.
 */
.macro ISR_NO_ERROR_CODE number
.global isr\number
.type isr\number, @function
isr\number:
  cli
  push $0
  push $\number
  jmp isr_common_stub
.size isr\number, . - isr\number
.endm

/**
 * @brief Defines a CPU exception stub for vectors with CPU error codes.
 *
 * The processor has already pushed the error code, so this macro only pushes
 * the interrupt vector number.
 */
.macro ISR_ERROR_CODE number
.global isr\number
.type isr\number, @function
isr\number:
  cli
  push $\number
  jmp isr_common_stub
.size isr\number, . - isr\number
.endm

/* CPU exception vectors 0-31. */
ISR_NO_ERROR_CODE 0
ISR_NO_ERROR_CODE 1
ISR_NO_ERROR_CODE 2
ISR_NO_ERROR_CODE 3
ISR_NO_ERROR_CODE 4
ISR_NO_ERROR_CODE 5
ISR_NO_ERROR_CODE 6
ISR_NO_ERROR_CODE 7
ISR_ERROR_CODE 8
ISR_NO_ERROR_CODE 9
ISR_ERROR_CODE 10
ISR_ERROR_CODE 11
ISR_ERROR_CODE 12
ISR_ERROR_CODE 13
ISR_ERROR_CODE 14
ISR_NO_ERROR_CODE 15
ISR_NO_ERROR_CODE 16
ISR_ERROR_CODE 17
ISR_NO_ERROR_CODE 18
ISR_NO_ERROR_CODE 19
ISR_NO_ERROR_CODE 20
ISR_NO_ERROR_CODE 21
ISR_NO_ERROR_CODE 22
ISR_NO_ERROR_CODE 23
ISR_NO_ERROR_CODE 24
ISR_NO_ERROR_CODE 25
ISR_NO_ERROR_CODE 26
ISR_NO_ERROR_CODE 27
ISR_NO_ERROR_CODE 28
ISR_ERROR_CODE 29
ISR_ERROR_CODE 30
ISR_NO_ERROR_CODE 31

/**
 * @brief Defines a hardware IRQ stub.
 *
 * IRQs do not provide CPU error codes, so a synthetic zero error code is
 * pushed before the remapped IDT vector number.
 */
.macro IRQ number vector
.global irq\number
.type irq\number, @function
irq\number:
  cli
  push $0
  push $\vector
  jmp irq_common_stub
.size irq\number, . - irq\number
.endm

/* Remapped 8259 PIC IRQ vectors 32-47. */
IRQ 0 32
IRQ 1 33
IRQ 2 34
IRQ 3 35
IRQ 4 36
IRQ 5 37
IRQ 6 38
IRQ 7 39
IRQ 8 40
IRQ 9 41
IRQ 10 42
IRQ 11 43
IRQ 12 44
IRQ 13 45
IRQ 14 46
IRQ 15 47

/**
 * @brief Common ISR path used by all CPU exception stubs.
 *
 * Saves general-purpose registers, switches to the kernel data segment,
 * calls the C-level isr_handler(), then restores state.
 */
isr_common_stub:
  /* Save all general-purpose registers in the order expected by C. */
  pusha

  /* Preserve the interrupted data segment selector. */
  mov %ds, %ax
  push %eax

  /* Use the kernel data selector while running the C handler. */
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  /* Pass a pointer to the saved interrupt frame. */
  push %esp
  call isr_handler
  add $4, %esp

  /* Restore the interrupted data segment selector. */
  pop %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  popa

  /* Drop interrupt number and error code before returning with iret. */
  add $8, %esp
  iret

/**
 * @brief Common IRQ path used by all hardware interrupt stubs.
 *
 * This mirrors the exception path but dispatches to irq_handler(), which also
 * acknowledges the interrupt at the PIC.
 */
irq_common_stub:
  pusha

  /* Preserve the interrupted data segment selector. */
  mov %ds, %ax
  push %eax

  /* Use the kernel data selector while running the C handler. */
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  /* Pass a pointer to the saved interrupt frame. */
  push %esp
  call irq_handler
  add $4, %esp

  /* Restore the interrupted data segment selector. */
  pop %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  popa

  /* Drop interrupt number and synthetic error code before returning. */
  add $8, %esp
  iret
