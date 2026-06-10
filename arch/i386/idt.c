#include <kernel/idt.h>
#include <kernel/serial.h>
#include <kernel/vga.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file idt.c
 * @brief Builds and loads the i386 Interrupt Descriptor Table.
 */

struct idt_entry {
  uint16_t base_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t flags;
  uint16_t base_high;
} __attribute__((packed));

struct idt_pointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct interrupt_frame {
  uint32_t ds;
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t interrupt_number;
  uint32_t error_code;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
} __attribute__((packed));

static struct idt_entry idt_entries[IDT_ENTRY_COUNT];
static struct idt_pointer idt_ptr;

extern void idt_flush(uint32_t idt_ptr_address);

/*
 * These are implemented in arch/i386/isr_stubs.s.
 */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/**
 * @brief Writes one interrupt gate into the IDT.
 *
 * @param index IDT vector number.
 * @param handler_address Address of the assembly ISR stub.
 * @param selector Kernel code segment selector.
 * @param flags Gate flags.
 */
static void idt_set_gate(size_t index, uint32_t handler_address,
                         uint16_t selector, uint8_t flags) {
  idt_entries[index].base_low = handler_address & 0xFFFF;
  idt_entries[index].selector = selector;
  idt_entries[index].zero = 0;
  idt_entries[index].flags = flags;
  idt_entries[index].base_high = (handler_address >> 16) & 0xFFFF;
}

/**
 * @brief Installs the first 32 CPU exception handlers.
 */
static void idt_install_cpu_exceptions(void) {
  idt_set_gate(0, (uint32_t)isr0, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(1, (uint32_t)isr1, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(2, (uint32_t)isr2, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(3, (uint32_t)isr3, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(4, (uint32_t)isr4, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(5, (uint32_t)isr5, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(6, (uint32_t)isr6, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(7, (uint32_t)isr7, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(8, (uint32_t)isr8, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(9, (uint32_t)isr9, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(10, (uint32_t)isr10, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(11, (uint32_t)isr11, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(12, (uint32_t)isr12, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(13, (uint32_t)isr13, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(14, (uint32_t)isr14, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(15, (uint32_t)isr15, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(16, (uint32_t)isr16, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(17, (uint32_t)isr17, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(18, (uint32_t)isr18, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(19, (uint32_t)isr19, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(20, (uint32_t)isr20, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(21, (uint32_t)isr21, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(22, (uint32_t)isr22, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(23, (uint32_t)isr23, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(24, (uint32_t)isr24, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(25, (uint32_t)isr25, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(26, (uint32_t)isr26, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(27, (uint32_t)isr27, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(28, (uint32_t)isr28, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(29, (uint32_t)isr29, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(30, (uint32_t)isr30, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
  idt_set_gate(31, (uint32_t)isr31, IDT_KERNEL_CODE_SELECTOR,
               IDT_INTERRUPT_GATE_RING0);
}

/**
 * @brief Initializes and loads the IDT.
 */
void idt_initialize(void) {
  idt_ptr.limit = sizeof(idt_entries) - 1;
  idt_ptr.base = (uint32_t)&idt_entries[0];

  for (size_t i = 0; i < IDT_ENTRY_COUNT; i++) {
    idt_set_gate(i, 0, 0, 0);
  }

  idt_install_cpu_exceptions();

  idt_flush((uint32_t)&idt_ptr);
}

/**
 * @brief C-level exception handler called by all ISR stubs.
 *
 * @param frame Saved CPU state pushed by the common ISR stub.
 */
void isr_handler(struct interrupt_frame* frame) {
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
  terminal_writestring("CPU exception caught.\n");

  serial_writestring("[wigOSX] CPU exception caught.\n");
  serial_writestring("[wigOSX] System halted.\n");

  (void)frame;

  while (1) {
    __asm__ volatile("cli; hlt");
  }
}