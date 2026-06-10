.section .text
.global idt_flush
.type idt_flush, @function

/**
 * @file idt_flush.s
 * @brief Assembly helper for loading the i386 IDT.
 */

/**
 * @brief Loads the IDT pointer into the CPU IDTR register.
 *
 * Stack layout:
 * - 0(%esp): return address
 * - 4(%esp): address of struct idt_pointer
 */
idt_flush:
    mov 4(%esp), %eax
    lidt (%eax)
    ret

.size idt_flush, . - idt_flush