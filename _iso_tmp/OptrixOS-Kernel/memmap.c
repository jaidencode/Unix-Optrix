#include "memmap.h"
#include <stdint.h>

static mem_region_t regions[MAX_MEM_REGIONS];
static size_t region_count = 0;

/* A very small stub that reports a single usable region starting at 1MB.
   Real boot loaders would query the BIOS using interrupt 0x15 (E820) before
   entering protected mode and pass the map to the kernel. */
void memmap_init(void) {
    regions[0].base = 0x100000;   /* 1MB */
    regions[0].length = 15 * 1024 * 1024; /* 15MB */
    regions[0].type = 1;
    region_count = 1;
}

size_t memmap_region_count(void) {
    return region_count;
}

const mem_region_t* memmap_get(size_t index) {
    if (index >= region_count)
        return 0;
    return &regions[index];
}

const mem_region_t* memmap_regions(void) {
    return regions;
}
