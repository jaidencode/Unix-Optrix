#include "pmm.h"
#include <stdint.h>
static uint8_t bitmap[128] = {0}; // 4096 frames (4MB)
void pmm_init(void) {
    for (int i=0; i<128; ++i) bitmap[i]=0;
}
void* pmm_alloc(void) {
    for (int i=0; i<1024; ++i) {
        int byte = i/8, bit = i%8;
        if (!(bitmap[byte] & (1<<bit))) {
            bitmap[byte] |= (1<<bit);
            return (void*)(i*0x1000 + 0x100000);
        }
    }
    return 0;
}
void pmm_free(void* p) {
    int i = (((uintptr_t)p) - 0x100000)/0x1000;
    bitmap[i/8] &= ~(1<<(i%8));
}
