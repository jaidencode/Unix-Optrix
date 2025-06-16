#include <stdint.h>
#include "handler.h"

#define WIDTH  1920
#define HEIGHT 1080

void kernel_main(uint32_t fb_addr) {
    uint32_t *fb = (uint32_t*)fb_addr;

    for (uint32_t i = 0; i < WIDTH * HEIGHT; ++i)
        fb[i] = 0x000000; // black

    for (uint32_t x = 0; x < WIDTH; ++x) {
        fb[x] = 0xFFFFFF;
        fb[(HEIGHT-1)*WIDTH + x] = 0xFFFFFF;
    }
    for (uint32_t y = 0; y < HEIGHT; ++y) {
        fb[y*WIDTH] = 0xFFFFFF;
        fb[y*WIDTH + WIDTH-1] = 0xFFFFFF;
    }

    sub_handler();
    while (1) { __asm__("hlt"); }
}
