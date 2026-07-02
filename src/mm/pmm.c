#include <kernel/boot/multiboot.h>
#include <kernel/core/memory.h>
#include <kernel/core/version.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <kernel/mm/pmm.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file pmm.c
 * @brief Stage 13 readable bitmap physical memory manager.
 *
 * This module tracks physical memory as 4 KiB frames.
 *
 * Bitmap meaning:
 * - bit = 0: frame is free
 * - bit = 1: frame is used or reserved
 *
 * Stage 13 deliberately keeps the bitmap static and simple. Later stages can
 * move the bitmap into a chosen physical memory region if needed.
 */

#define PMM_FRAME_SIZE_U64 4096ULL
#define PMM_LOW_MEMORY_RESERVED_BYTES 0x00100000ULL

/*
 * i386 without PAE can address up to 4 GiB of physical memory.
 *
 * 4 GiB / 4 KiB = 1,048,576 frames.
 * 1,048,576 bits = 131,072 bytes = 128 KiB bitmap.
 */
#define PMM_MAX_PHYSICAL_MEMORY_BYTES 0x100000000ULL
#define PMM_MAX_FRAMES \
  ((uint32_t)(PMM_MAX_PHYSICAL_MEMORY_BYTES / PMM_FRAME_SIZE_U64))

#define PMM_BITS_PER_WORD 32U
#define PMM_BITMAP_WORD_COUNT (PMM_MAX_FRAMES / PMM_BITS_PER_WORD)

/*
 * These symbols are provided by linker/kernel.ld.
 *
 * Because paging is not enabled yet, the linked kernel addresses are also the
 * physical addresses we need to reserve.
 */
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

static uint32_t pmm_bitmap[PMM_BITMAP_WORD_COUNT];
static uint32_t pmm_allocated_bitmap[PMM_BITMAP_WORD_COUNT];

static bool pmm_initialized = false;
static uint32_t pmm_total_frames = 0;
static uint32_t pmm_free_frames = 0;
static uint32_t pmm_used_frames = 0;
static uint32_t pmm_next_search_frame = 0;

static bool pmm_is_frame_allocated(uint32_t frame_index) {
  uint32_t word_index = pmm_bitmap_word_index(frame_index);
  uint32_t bit_mask = pmm_bitmap_bit_mask(frame_index);

  return (pmm_allocated_bitmap[word_index] & bit_mask) != 0;
}

static void pmm_mark_frame_allocated(uint32_t frame_index) {
  if (!pmm_is_valid_frame_index(frame_index)) {
    return;
  }

  pmm_allocated_bitmap[pmm_bitmap_word_index(frame_index)] |=
      pmm_bitmap_bit_mask(frame_index);
}

static void pmm_mark_frame_unallocated(uint32_t frame_index) {
  if (!pmm_is_valid_frame_index(frame_index)) {
    return;
  }

  pmm_allocated_bitmap[pmm_bitmap_word_index(frame_index)] &=
      ~pmm_bitmap_bit_mask(frame_index);
}

static void pmm_putchar(char c) {
  terminal_putchar(c);

  if (serial_is_initialized()) {
    serial_putchar(c);
  }
}

static void pmm_writestring(const char* text) {
  terminal_writestring(text);

  if (serial_is_initialized()) {
    serial_writestring(text);
  }
}

static void pmm_print_uint32(uint32_t value) {
  char digits[10];
  size_t digit_count = 0;

  if (value == 0) {
    pmm_putchar('0');
    return;
  }

  while (value > 0) {
    digits[digit_count] = '0' + (value % 10);
    value = value / 10;
    digit_count++;
  }

  while (digit_count > 0) {
    digit_count--;
    pmm_putchar(digits[digit_count]);
  }
}

static uint64_t pmm_align_down(uint64_t value) {
  return value & ~(PMM_FRAME_SIZE_U64 - 1ULL);
}

static uint64_t pmm_align_up(uint64_t value) {
  uint64_t mask = PMM_FRAME_SIZE_U64 - 1ULL;

  if ((value & mask) == 0) {
    return value;
  }

  if (value > UINT64_MAX - mask) {
    return UINT64_MAX & ~mask;
  }

  return (value + mask) & ~mask;
}

static uint64_t pmm_range_end(uint64_t base_address, uint64_t length) {
  if (base_address > UINT64_MAX - length) {
    return UINT64_MAX;
  }

  return base_address + length;
}

static bool pmm_is_valid_frame_index(uint32_t frame_index) {
  return frame_index < pmm_total_frames && frame_index < PMM_MAX_FRAMES;
}

static uint32_t pmm_bitmap_word_index(uint32_t frame_index) {
  return frame_index / PMM_BITS_PER_WORD;
}

static uint32_t pmm_bitmap_bit_mask(uint32_t frame_index) {
  return 1U << (frame_index % PMM_BITS_PER_WORD);
}

static bool pmm_is_frame_index_used(uint32_t frame_index) {
  uint32_t word_index = pmm_bitmap_word_index(frame_index);
  uint32_t bit_mask = pmm_bitmap_bit_mask(frame_index);

  return (pmm_bitmap[word_index] & bit_mask) != 0;
}

static void pmm_mark_frame_used(uint32_t frame_index) {
  if (!pmm_is_valid_frame_index(frame_index)) {
    return;
  }

  if (pmm_is_frame_index_used(frame_index)) {
    return;
  }

  pmm_bitmap[pmm_bitmap_word_index(frame_index)] |=
      pmm_bitmap_bit_mask(frame_index);

  if (pmm_free_frames > 0) {
    pmm_free_frames--;
  }

  pmm_used_frames++;
}

static void pmm_mark_frame_free(uint32_t frame_index) {
  if (!pmm_is_valid_frame_index(frame_index)) {
    return;
  }

  if (!pmm_is_frame_index_used(frame_index)) {
    return;
  }

  pmm_bitmap[pmm_bitmap_word_index(frame_index)] &=
      ~pmm_bitmap_bit_mask(frame_index);

  pmm_free_frames++;

  if (pmm_used_frames > 0) {
    pmm_used_frames--;
  }
}

/**
 * @brief Marks every full frame inside a usable range as free.
 *
 * Partial frames at the edges are not freed because the whole 4 KiB frame is
 * not guaranteed to be safe.
 */
static void pmm_mark_range_free(uint64_t base_address, uint64_t length) {
  uint64_t end_address = pmm_range_end(base_address, length);
  uint64_t first_frame = pmm_align_up(base_address) / PMM_FRAME_SIZE_U64;
  uint64_t last_frame = pmm_align_down(end_address) / PMM_FRAME_SIZE_U64;

  if (first_frame >= pmm_total_frames) {
    return;
  }

  if (last_frame > pmm_total_frames) {
    last_frame = pmm_total_frames;
  }

  for (uint64_t frame = first_frame; frame < last_frame; frame++) {
    pmm_mark_frame_free((uint32_t)frame);
  }
}

/**
 * @brief Marks every frame touched by a reserved range as used.
 *
 * Partial frames are reserved completely to avoid giving out memory that partly
 * overlaps with kernel or bootloader data.
 */
static void pmm_mark_range_used(uint64_t base_address, uint64_t length) {
  uint64_t end_address = pmm_range_end(base_address, length);
  uint64_t first_frame = pmm_align_down(base_address) / PMM_FRAME_SIZE_U64;
  uint64_t last_frame = pmm_align_up(end_address) / PMM_FRAME_SIZE_U64;

  if (first_frame >= pmm_total_frames) {
    return;
  }

  if (last_frame > pmm_total_frames) {
    last_frame = pmm_total_frames;
  }

  for (uint64_t frame = first_frame; frame < last_frame; frame++) {
    pmm_mark_frame_used((uint32_t)frame);
  }
}

static uint64_t pmm_find_highest_physical_address(void) {
  uint64_t highest_address = 0;
  uint32_t region_count = memory_get_region_count();

  for (uint32_t i = 0; i < region_count; i++) {
    const memory_region_t* region = memory_get_region(i);

    if (region == NULL) {
      continue;
    }

    uint64_t end_address = pmm_range_end(region->base_address, region->length);

    if (end_address > highest_address) {
      highest_address = end_address;
    }
  }

  if (highest_address > PMM_MAX_PHYSICAL_MEMORY_BYTES) {
    highest_address = PMM_MAX_PHYSICAL_MEMORY_BYTES;
  }

  return pmm_align_up(highest_address);
}

static void pmm_reset_bitmap(void) {
  for (uint32_t i = 0; i < PMM_BITMAP_WORD_COUNT; i++) {
    pmm_bitmap[i] = 0xFFFFFFFFU;
    pmm_allocated_bitmap[i] = 0;
  }
}

static void pmm_release_usable_memory_regions(void) {
  uint32_t region_count = memory_get_region_count();

  for (uint32_t i = 0; i < region_count; i++) {
    const memory_region_t* region = memory_get_region(i);

    if (region == NULL) {
      continue;
    }

    if (region->type != MEMORY_REGION_USABLE) {
      continue;
    }

    pmm_mark_range_free(region->base_address, region->length);
  }
}

static void pmm_reserve_low_memory(void) {
  pmm_mark_range_used(0, PMM_LOW_MEMORY_RESERVED_BYTES);
}

static void pmm_reserve_kernel_image(void) {
  uintptr_t kernel_start = (uintptr_t)&_kernel_start;
  uintptr_t kernel_end = (uintptr_t)&_kernel_end;

  if (kernel_end <= kernel_start) {
    return;
  }

  pmm_mark_range_used((uint64_t)kernel_start,
                      (uint64_t)(kernel_end - kernel_start));
}

static void pmm_reserve_bitmap_storage(void) {
  uintptr_t bitmap_start = (uintptr_t)pmm_bitmap;
  uintptr_t bitmap_end = bitmap_start + sizeof(pmm_bitmap);

  pmm_mark_range_used((uint64_t)bitmap_start,
                      (uint64_t)(bitmap_end - bitmap_start));
}

static void pmm_reserve_multiboot_information(uint32_t multiboot_info_address) {
  if (multiboot_info_address == 0) {
    return;
  }

  const multiboot_info_t* info =
      (const multiboot_info_t*)multiboot_info_address;

  pmm_mark_range_used(multiboot_info_address, sizeof(multiboot_info_t));

  if ((info->flags & MULTIBOOT_INFO_MEMORY_MAP) != 0 && info->mmap_addr != 0 &&
      info->mmap_length > 0) {
    pmm_mark_range_used(info->mmap_addr, info->mmap_length);
  }
}

void pmm_initialize(uint32_t multiboot_info_address) {
  pmm_initialized = false;
  pmm_total_frames = 0;
  pmm_free_frames = 0;
  pmm_used_frames = 0;
  pmm_next_search_frame = 0;

  pmm_reset_bitmap();

  uint64_t highest_address = pmm_find_highest_physical_address();

  pmm_total_frames = (uint32_t)(highest_address / PMM_FRAME_SIZE_U64);
  pmm_used_frames = pmm_total_frames;
  pmm_free_frames = 0;

  pmm_release_usable_memory_regions();

  /*
   * Re-reserve unsafe areas after freeing usable RAM.
   *
   * This makes the initialization order easy to understand:
   * 1. Everything starts reserved.
   * 2. Usable memory-map regions become free.
   * 3. Kernel/bootloader-owned areas become reserved again.
   */
  pmm_reserve_low_memory();
  pmm_reserve_kernel_image();
  pmm_reserve_bitmap_storage();
  pmm_reserve_multiboot_information(multiboot_info_address);

  pmm_initialized = true;
}

bool pmm_allocate_frame(uint32_t* out_physical_address) {
  if (!pmm_initialized || out_physical_address == NULL) {
    return false;
  }

  for (uint32_t frame = pmm_next_search_frame; frame < pmm_total_frames;
       frame++) {
    if (!pmm_is_frame_index_used(frame)) {
      pmm_mark_frame_used(frame);
      pmm_mark_frame_allocated(frame);

      *out_physical_address = frame * PMM_FRAME_SIZE;
      pmm_next_search_frame = frame + 1;
      return true;
    }
  }

  for (uint32_t frame = 0; frame < pmm_next_search_frame; frame++) {
    if (!pmm_is_frame_index_used(frame)) {
      pmm_mark_frame_used(frame);
      pmm_mark_frame_allocated(frame);

      *out_physical_address = frame * PMM_FRAME_SIZE;
      pmm_next_search_frame = frame + 1;
      return true;
    }
  }

  return false;
}

bool pmm_free_frame(uint32_t physical_address) {
  if (!pmm_initialized) {
    return false;
  }

  if ((physical_address % PMM_FRAME_SIZE) != 0) {
    return false;
  }

  uint32_t frame_index = physical_address / PMM_FRAME_SIZE;

  if (!pmm_is_valid_frame_index(frame_index)) {
    return false;
  }

  if (!pmm_is_frame_allocated(frame_index)) {
    return false;
  }

  pmm_mark_frame_unallocated(frame_index);
  pmm_mark_frame_free(frame_index);

  if (frame_index < pmm_next_search_frame) {
    pmm_next_search_frame = frame_index;
  }

  return true;
}

bool pmm_is_frame_used(uint32_t physical_address) {
  uint32_t frame_index = physical_address / PMM_FRAME_SIZE;

  if (!pmm_is_valid_frame_index(frame_index)) {
    return true;
  }

  return pmm_is_frame_index_used(frame_index);
}

uint32_t pmm_get_total_frame_count(void) { return pmm_total_frames; }

uint32_t pmm_get_free_frame_count(void) { return pmm_free_frames; }

uint32_t pmm_get_used_frame_count(void) { return pmm_used_frames; }

void pmm_print_summary(void) {
  pmm_writestring(WIGOSX_STAGE_LABEL);
  pmm_writestring(".\n");

  pmm_writestring("PMM frame size: ");
  pmm_print_uint32(PMM_FRAME_SIZE);
  pmm_writestring(" bytes\n");

  pmm_writestring("Tracked frames: ");
  pmm_print_uint32(pmm_total_frames);
  pmm_putchar('\n');

  pmm_writestring("Free frames: ");
  pmm_print_uint32(pmm_free_frames);
  pmm_putchar('\n');

  pmm_writestring("Used/reserved frames: ");
  pmm_print_uint32(pmm_used_frames);
  pmm_putchar('\n');
}

bool pmm_run_basic_self_test(void) {
  uint32_t free_before = pmm_get_free_frame_count();
  uint32_t used_before = pmm_get_used_frame_count();
  uint32_t physical_address = 0;

  if (!pmm_allocate_frame(&physical_address)) {
    return false;
  }

  if (!pmm_is_frame_used(physical_address)) {
    return false;
  }

  if (pmm_get_free_frame_count() != free_before - 1) {
    return false;
  }

  if (pmm_get_used_frame_count() != used_before + 1) {
    return false;
  }

  if (!pmm_free_frame(physical_address)) {
    return false;
  }

  if (pmm_get_free_frame_count() != free_before) {
    return false;
  }

  if (pmm_get_used_frame_count() != used_before) {
    return false;
  }

  return true;
}
