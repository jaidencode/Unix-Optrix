#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
void put_pixel(int x, int y, uint8_t color);
uint8_t get_pixel(int x, int y);
void draw_rect(int x, int y, int w, int h, uint8_t color);
void draw_rounded_rect(int x, int y, int w, int h, int r, uint8_t color);
void graphics_set_framebuffer(uint32_t addr);
#endif
