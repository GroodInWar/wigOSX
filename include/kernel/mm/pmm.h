#ifndef KERNEL_CORE_PMM_H
#define KERNEL_CORE_PMM_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file pmm.h
 * @brief Stage 13 bitmap physical memory manager.
 *
 * The PMM tracks physical 4 KiB frames. It does not provide malloc(),
 * virtual memory, paging, or variable-sized allocation yet.
 */

#define PMM_FRAME_SIZE 4096U

/**
 * @brief Initializes the physical memory manager from the normalized memory
 * map.
 *
 * @param multiboot_info_address Physical address of the Multiboot information
 * structure. Stage 13 uses this only to reserve bootloader-owned metadata.
 */
void pmm_initialize(uint32_t multiboot_info_address);

/**
 * @brief Prints a readable PMM summary.
 */
void pmm_print_summary(void);

/**
 * @brief Allocates one free 4 KiB physical frame.
 *
 * @param out_physical_address Receives the physical address of the allocated
 * frame.
 *
 * @return true if a frame was allocated, false if no frame was available.
 */
bool pmm_allocate_frame(uint32_t* out_physical_address);

/**
 * @brief Frees one previously allocated 4 KiB physical frame.
 *
 * @param physical_address 4 KiB-aligned physical frame address.
 *
 * @return true if the frame was freed, false if the address was invalid.
 */
bool pmm_free_frame(uint32_t physical_address);

/**
 * @brief Returns true if the frame containing physical_address is marked used.
 */
bool pmm_is_frame_used(uint32_t physical_address);

/**
 * @brief Returns the total number of tracked physical frames.
 */
uint32_t pmm_get_total_frame_count(void);

/**
 * @brief Returns the number of free physical frames.
 */
uint32_t pmm_get_free_frame_count(void);

/**
 * @brief Returns the number of used/reserved physical frames.
 */
uint32_t pmm_get_used_frame_count(void);

#endif /* KERNEL_CORE_PMM_H */