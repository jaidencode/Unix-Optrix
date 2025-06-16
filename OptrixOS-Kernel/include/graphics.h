#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
void put_pixel(int x, int y, uint8_t color);
void draw_rect(int x, int y, int w, int h, uint8_t color);
#endif
