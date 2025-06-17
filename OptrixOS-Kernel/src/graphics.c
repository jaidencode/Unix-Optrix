#include "graphics.h"
#include <stdint.h>

#define WIDTH 800
#define HEIGHT 600
static volatile uint8_t *VGA = (uint8_t *)0xA0000;
static uint8_t *BACKBUF = 0;

void graphics_set_framebuffer(uint32_t addr) {
    VGA = (volatile uint8_t *)addr;
}

void graphics_set_backbuffer(uint8_t *buf) {
    BACKBUF = buf;
}

void graphics_present(void) {
    if(!BACKBUF) return;
    for(int i=0;i<WIDTH*HEIGHT;i++)
        VGA[i] = BACKBUF[i];
}

void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
    if(BACKBUF)
        BACKBUF[y * WIDTH + x] = color;
    else
        VGA[y * WIDTH + x] = color;
}

uint8_t get_pixel(int x, int y) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return 0;
    if(BACKBUF)
        return BACKBUF[y * WIDTH + x];
    return VGA[y * WIDTH + x];
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            put_pixel(x + i, y + j, color);
        }
    }
}

void draw_rounded_rect(int x, int y, int w, int h, int r, uint8_t color) {
    if(r <= 0) { draw_rect(x, y, w, h, color); return; }
    /* central rects */
    draw_rect(x + r, y, w - 2*r, h, color);
    draw_rect(x, y + r, r, h - 2*r, color);
    draw_rect(x + w - r, y + r, r, h - 2*r, color);

    for(int dy = 0; dy < r; dy++) {
        for(int dx = 0; dx < r; dx++) {
            if(dx*dx + dy*dy <= r*r) {
                put_pixel(x + r - dx - 1, y + r - dy - 1, color); /* TL */
                put_pixel(x + w - r + dx, y + r - dy - 1, color);  /* TR */
                put_pixel(x + r - dx - 1, y + h - r + dy, color);  /* BL */
                put_pixel(x + w - r + dx, y + h - r + dy, color);  /* BR */
            }
        }
    }
}
