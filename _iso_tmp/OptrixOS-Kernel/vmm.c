#include "vmm.h"
#include "pmm.h"
#include <stdint.h>

/* Very small identity mapping paging setup. Only enables paging with a single
   page directory and page table covering the first 4MB. */

#define PAGE_SIZE 4096
static uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t first_table[1024] __attribute__((aligned(PAGE_SIZE)));

void vmm_init(void) {
    for (int i = 0; i < 1024; i++) {
        first_table[i] = (i * PAGE_SIZE) | 3; /* Present, RW */
    }
    page_directory[0] = ((uint32_t)first_table) | 3;
    for (int i = 1; i < 1024; i++)
        page_directory[i] = 0;

    /* Load page directory */
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; /* set PG bit */
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}
