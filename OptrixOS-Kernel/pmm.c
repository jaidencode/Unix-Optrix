#include "pmm.h"
#include <stdint.h>

#define MAX_FRAMES 4096
#define FRAME_SIZE 4096

static uint8_t bitmap[MAX_FRAMES / 8];
static size_t total_frames = 0;

static inline void set_bit(size_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(size_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int test_bit(size_t bit) {
    return bitmap[bit / 8] & (1 << (bit % 8));
}

void pmm_init(const mem_region_t* regions, size_t region_count) {
    total_frames = 0;
    for (size_t i = 0; i < MAX_FRAMES / 8; i++)
        bitmap[i] = 0;

    for (size_t i = 0; i < region_count && total_frames < MAX_FRAMES; i++) {
        if (regions[i].type != 1)
            continue;
        uint32_t start = regions[i].base;
        uint32_t end = start + regions[i].length;
        for (uint32_t addr = start; addr + FRAME_SIZE <= end && total_frames < MAX_FRAMES; addr += FRAME_SIZE) {
            clear_bit(total_frames);
            total_frames++;
        }
    }
}

void* pmm_alloc(void) {
    for (size_t i = 0; i < total_frames; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            return (void*)(i * FRAME_SIZE);
        }
    }
    return 0;
}

void pmm_free(void* frame) {
    size_t idx = (uintptr_t)frame / FRAME_SIZE;
    if (idx < total_frames)
        clear_bit(idx);
}
