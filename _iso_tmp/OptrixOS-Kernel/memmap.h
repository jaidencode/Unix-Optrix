#ifndef MEMMAP_H
#define MEMMAP_H

#include <stddef.h>
#include <stdint.h>

#define MAX_MEM_REGIONS 16

typedef struct {
    uint32_t base;
    uint32_t length;
    uint32_t type; /* 1=usable */
} mem_region_t;

void memmap_init(void);
size_t memmap_region_count(void);
const mem_region_t* memmap_get(size_t index);
const mem_region_t* memmap_regions(void);

#endif /* MEMMAP_H */
