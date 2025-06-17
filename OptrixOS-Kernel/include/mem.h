#ifndef MEM_H
#define MEM_H
#include <stddef.h>

void mem_init(unsigned char *base, size_t size);
void* mem_alloc(size_t size);
size_t mem_total(void);
size_t mem_used(void);
size_t mem_free(void);

#endif
