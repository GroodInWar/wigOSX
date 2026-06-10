.section .text

.extern isr_handler

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

.macro ISR_ERROR_CODE number
.global isr\number
.type isr\number, @function
isr\number:
  cli
  push $\number
  jmp isr_common_stub
.size isr\number, . - isr\number
.endm

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
 * @brief Common ISR path used by all CPU exception stubs.
 *
 * Saves general-purpose registers, switches to the kernel data segment,
 * calls the C-level isr_handler(), then restores state.
 */
isr_common_stub:
  pusha

  mov %ds, %ax
  push %eax

  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  push %esp
  call isr_handler
  add $4, %esp

  pop %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  popa

  add $8, %esp
  iret