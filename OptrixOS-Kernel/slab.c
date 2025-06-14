#include "slab.h"
#include "heap.h"

void slab_init(void) {
    /* Placeholder: our simple allocator uses the kernel heap directly */
}

void* kmalloc(size_t size) {
    return heap_alloc(size);
}

void kfree(void* ptr) {
    heap_free(ptr);
}
