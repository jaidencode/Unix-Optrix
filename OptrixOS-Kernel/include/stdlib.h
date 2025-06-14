// Minimal stdlib.h for freestanding kernels - OptrixOS
#pragma once

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned long size_t;

void* malloc(size_t size);  // You can just return NULL or use your allocator
void  free(void* ptr);      // No-op for now
