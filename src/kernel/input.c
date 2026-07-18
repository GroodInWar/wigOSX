#include <kernel/core/input.h>
#include <kernel/drivers/keyboard.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file input.c
 * @brief Interrupt-safe queued input service.
 */

/**
 * @brief Number of translated characters retained by the queue.
 *
 * The capacity must remain a power of two so queue positions can use a mask
 * instead of division.
 */
#define INPUT_QUEUE_CAPACITY 64U

/**
 * @brief Converts a monotonically increasing queue position into an array
 * index.
 */
#define INPUT_QUEUE_INDEX_MASK (INPUT_QUEUE_CAPACITY - 1U)

#if (INPUT_QUEUE_CAPACITY == 0U) || \
    ((INPUT_QUEUE_CAPACITY & (INPUT_QUEUE_CAPACITY - 1U)) != 0U)
#error "INPUT_QUEUE_CAPACITY must be a nonzero power of two"
#endif

/**
 * @brief Character storage shared by the IRQ producer and foreground consumer.
 */
static volatile char input_queue[INPUT_QUEUE_CAPACITY];

/**
 * @brief Position where the interrupt producer writes the next character.
 */
static volatile uint32_t input_queue_head = 0;

/**
 * @brief Position where the foreground consumer reads the next character.
 */
static volatile uint32_t input_queue_tail = 0;

/**
 * @brief Saturating count of characters discarded because the queue was full.
 */
static volatile uint32_t input_dropped_character_count = 0;

/**
 * @brief Initializes all input queue state.
 */
void input_initialize(void) {
  input_queue_head = 0;
  input_queue_tail = 0;
  input_dropped_character_count = 0;
}

/**
 * @brief Attempts to append one character to the IRQ input queue.
 */
static bool input_enqueue_character(char ascii) {
  uint32_t head = input_queue_head;
  uint32_t tail = input_queue_tail;

  if ((head - tail) >= INPUT_QUEUE_CAPACITY) {
    uint32_t dropped = input_dropped_character_count;

    if (dropped != UINT32_MAX) {
      input_dropped_character_count = dropped + 1U;
    }

    return false;
  }

  input_queue[head & INPUT_QUEUE_INDEX_MASK] = ascii;
  input_queue_head = head + 1U;

  return true;
}

/**
 * @brief Reads a translated character from the keyboard and queues it.
 */
void input_handle_keyboard_interrupt(void) {
  char ascii = '\0';

  if (keyboard_handle_interrupt(&ascii)) {
    (void)input_enqueue_character(ascii);
  }
}

/**
 * @brief Removes one queued character for foreground processing.
 */
bool input_try_read_character(char* out_ascii) {
  if (out_ascii == NULL) {
    return false;
  }

  uint32_t tail = input_queue_tail;
  uint32_t head = input_queue_head;

  if (tail == head) {
    return false;
  }

  char ascii = input_queue[tail & INPUT_QUEUE_INDEX_MASK];

  input_queue_tail = tail + 1U;
  *out_ascii = ascii;

  return true;
}

/**
 * @brief Returns the current number of queued characters.
 */
uint32_t input_get_pending_character_count(void) {
  uint32_t head = input_queue_head;
  uint32_t tail = input_queue_tail;

  return head - tail;
}

/**
 * @brief Returns the number of characters discarded due to queue overflow.
 */
uint32_t input_get_dropped_character_count(void) {
  return input_dropped_character_count;
}