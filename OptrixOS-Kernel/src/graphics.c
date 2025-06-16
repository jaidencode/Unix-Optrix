#include "graphics.h"
#include <stdint.h>

#define WIDTH 1920
#define HEIGHT 1080
static volatile uint8_t *VGA = (uint8_t *)0xA0000;

void graphics_set_framebuffer(uint32_t addr) {
    VGA = (volatile uint8_t *)addr;
}

void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
    VGA[y * WIDTH + x] = color;
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            put_pixel(x + i, y + j, color);
        }
    }
}
