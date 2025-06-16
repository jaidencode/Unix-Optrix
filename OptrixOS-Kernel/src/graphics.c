#include "graphics.h"
#include <stdint.h>

int FB_WIDTH = 0;
int FB_HEIGHT = 0;
int FB_BPP = 1;
static volatile uint8_t *VGA = (uint8_t *)0xA0000;

void graphics_set_framebuffer(uint32_t addr) {
    VGA = (volatile uint8_t *)addr;
}

void graphics_init(int width, int height, int bpp) {
    FB_WIDTH = width;
    FB_HEIGHT = height;
    FB_BPP = bpp / 8;
    if(FB_BPP <= 0) FB_BPP = 1;
}

void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT)
        return;
    uint32_t offset = (y * FB_WIDTH + x) * FB_BPP;
    if(FB_BPP == 1) {
        VGA[offset] = color;
    } else if(FB_BPP == 2) {
        ((uint16_t*)VGA)[y * FB_WIDTH + x] = color;
    } else {
        static const uint32_t palette[16] = {
            0x000000,0x0000AA,0x00AA00,0x00AAAA,
            0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
            0x555555,0x5555FF,0x55FF55,0x55FFFF,
            0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF
        };
        ((uint32_t*)VGA)[y * FB_WIDTH + x] = palette[color & 0x0F];
    }
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            put_pixel(x + i, y + j, color);
        }
    }
}
