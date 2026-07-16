#ifndef KERNEL_CORE_PANIC_H
#define KERNEL_CORE_PANIC_H

/**
 * @file panic.h
 * @brief Central fatal-error interface.
 */

/**
 * @brief Reports a fatal kernel error and halts forever.
 *
 * @param message Human-readable panic message. NULL uses a default message.
 */
void kernel_panic(const char* message) __attribute__((noreturn));

#endif /* KERNEL_CORE_PANIC_H */
