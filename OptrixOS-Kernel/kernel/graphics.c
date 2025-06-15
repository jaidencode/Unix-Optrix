#include "graphics.h"
#include "font8x8_basic.h"

#define VIDEO_MEMORY ((volatile uint8_t*)0xA0000)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void graphics_init(void) {
    (void)inb; (void)outb; // mode set in bootloader
}

void graphics_clear(uint8_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
        VIDEO_MEMORY[i] = color;
}

void graphics_draw_rect(int x, int y, int w, int h, uint8_t color) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            VIDEO_MEMORY[(y + j) * SCREEN_WIDTH + (x + i)] = color;
        }
    }
}

void graphics_draw_char(int x, int y, char c, uint8_t color) {
    const unsigned char *glyph = font8x8_basic[(unsigned char)c];
    for (int row = 0; row < 8; ++row) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < 8; ++col) {
            if (bits & (1 << col)) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                    VIDEO_MEMORY[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}

void graphics_draw_string(int x, int y, const char* s, uint8_t color) {
    while (*s) {
        graphics_draw_char(x, y, *s++, color);
        x += 8;
    }
}
