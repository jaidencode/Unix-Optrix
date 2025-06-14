#include "heap.h"
#include <stdint.h>

/* Very small bump pointer heap located after the kernel. */
extern uint8_t end; /* Provided by linker */
static uint8_t* heap_start = &end;
static uint8_t* heap_end   = (uint8_t*)0x800000; /* 8MB */
static uint8_t* current    = 0;

void heap_init(void) {
    current = heap_start;
}

void* heap_alloc(size_t size) {
    if (!current)
        heap_init();
    if (current + size >= heap_end)
        return 0;
    void* ret = current;
    current += size;
    return ret;
}

void heap_free(void* ptr) {
    /* simple bump allocator cannot free */
    (void)ptr;
}
