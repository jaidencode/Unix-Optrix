#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>
#include "memmap.h"

void pmm_init(const mem_region_t* regions, size_t region_count);
void* pmm_alloc(void);
void pmm_free(void* frame);

#endif /* PMM_H */
