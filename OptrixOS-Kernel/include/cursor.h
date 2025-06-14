#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>
#include <stddef.h>

// Basic Cursor structure, extend as needed
typedef struct {
    int width;
    int height;
    int hotspot_x;
    int hotspot_y;
    uint32_t* pixels;   // 32bpp ARGB, pointer to pixel data
} Cursor;

// Loads a .cur file from memory buffer
Cursor* load_cursor_from_cur(const uint8_t* data, size_t size);

// Draws the cursor at (x, y) on the framebuffer
void draw_cursor(uint32_t* fb, int fb_w, int fb_h, const Cursor* cur, int x, int y);

#endif // CURSOR_H
