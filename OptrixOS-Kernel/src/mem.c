#include "mem.h"

static unsigned char *heap_base = 0;
static size_t heap_size = 0;
static size_t heap_used = 0;

void mem_init(unsigned char *base, size_t size) {
    heap_base = base;
    heap_size = size;
    heap_used = 0;
}

void* mem_alloc(size_t size) {
    if(!heap_base || heap_used + size > heap_size)
        return 0;
    void* ptr = heap_base + heap_used;
    heap_used += size;
    return ptr;
}
size_t mem_total(void) { return heap_size; }
size_t mem_used(void) { return heap_used; }
size_t mem_free(void) { return heap_size > heap_used ? heap_size - heap_used : 0; }

