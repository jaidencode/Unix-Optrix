#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
void graphics_set_framebuffer(uint32_t addr);
void graphics_init(int width, int height, int bpp);
void put_pixel(int x, int y, uint8_t color);
void draw_rect(int x, int y, int w, int h, uint8_t color);
extern int FB_WIDTH;
extern int FB_HEIGHT;
extern int FB_BPP;
#endif
