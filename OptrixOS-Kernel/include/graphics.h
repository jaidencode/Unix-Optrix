#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

void graphics_init(void);
void graphics_clear(uint8_t color);
void graphics_draw_rect(int x, int y, int w, int h, uint8_t color);
void graphics_draw_char(int x, int y, char c, uint8_t color);
void graphics_draw_string(int x, int y, const char* s, uint8_t color);

#endif // GRAPHICS_H
