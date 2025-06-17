#include "graphics.h"
#include "screen.h"
#include <stdint.h>

static volatile uint8_t *VGA = (uint8_t *)0xA0000;
static uint8_t backbuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

static void flush_rect(int x, int y, int w, int h) {
    if(x < 0) { w += x; x = 0; }
    if(y < 0) { h += y; y = 0; }
    if(x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if(y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            VGA[(y+j)*SCREEN_WIDTH + (x+i)] =
                backbuffer[(y+j)*SCREEN_WIDTH + (x+i)];
        }
    }
}

void graphics_flush(int x, int y, int w, int h) {
    flush_rect(x, y, w, h);
}

void graphics_set_framebuffer(uint32_t addr) {
    VGA = (volatile uint8_t *)addr;
    flush_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;
    backbuffer[y * SCREEN_WIDTH + x] = color;
}

uint8_t get_pixel(int x, int y) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return 0;
    return backbuffer[y * SCREEN_WIDTH + x];
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            put_pixel(x + i, y + j, color);
        }
    }
    flush_rect(x, y, w, h);
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
    flush_rect(x, y, w, h);
}
