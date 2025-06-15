// Minimal stdlib.h for freestanding kernels - OptrixOS
#pragma once

#ifndef NULL
#define NULL ((void*)0)
#endif

#include <stddef.h>

void* malloc(size_t size);  // You can just return NULL or use your allocator
void  free(void* ptr);      // No-op for now
