#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
void graphics_init(void);
void graphics_fill_rect(int x, int y, int w, int h, uint8_t color);
#endif
