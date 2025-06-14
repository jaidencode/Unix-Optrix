#include "slab.h"
#include "heap.h"

typedef struct slab_node {
    struct slab_node* next;
} slab_node_t;

static slab_node_t* free_list = 0;

void slab_init(void) {
    free_list = 0;
}

void* kmalloc(size_t size) {
    if (size < sizeof(slab_node_t))
        size = sizeof(slab_node_t);

    if (free_list) {
        slab_node_t* node = free_list;
        free_list = node->next;
        return node;
    }

    return heap_alloc(size);
}

void kfree(void* ptr) {
    if (!ptr)
        return;

    slab_node_t* node = (slab_node_t*)ptr;
    node->next = free_list;
    free_list = node;
}
