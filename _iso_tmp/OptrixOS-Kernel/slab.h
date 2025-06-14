#ifndef SLAB_H
#define SLAB_H

#include <stddef.h>

void slab_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif /* SLAB_H */
