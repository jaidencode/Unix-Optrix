#include "cursor.h"
#include <stdint.h>
#include <stddef.h>
#ifndef NULL
#define NULL ((void*)0)
#endif

#define CURSOR_DUMMY_W 16
#define CURSOR_DUMMY_H 16

static uint32_t dummy_cursor_data[CURSOR_DUMMY_W * CURSOR_DUMMY_H];

static void fill_dummy_cursor() {
    for (int y = 0; y < CURSOR_DUMMY_H; ++y) {
        for (int x = 0; x < CURSOR_DUMMY_W; ++x) {
            if (x == 0 || y == 0 || x == CURSOR_DUMMY_W - 1 || y == CURSOR_DUMMY_H - 1)
                dummy_cursor_data[y * CURSOR_DUMMY_W + x] = 0xFF000000; // Black
            else
                dummy_cursor_data[y * CURSOR_DUMMY_W + x] = 0xFFFFFFFF; // White
        }
    }
}

Cursor* load_cursor_from_cur(const uint8_t* data, size_t size) {
    (void)data; (void)size;
    static Cursor dummy;
    dummy.width = CURSOR_DUMMY_W;
    dummy.height = CURSOR_DUMMY_H;
    dummy.hotspot_x = 0;
    dummy.hotspot_y = 0;
    fill_dummy_cursor();
    dummy.pixels = dummy_cursor_data;
    return &dummy;
}

void draw_cursor(uint32_t* fb, int fb_w, int fb_h, const Cursor* cur, int x, int y) {
    if (!fb || !cur || !cur->pixels) return;
    for (int cy = 0; cy < cur->height; ++cy) {
        for (int cx = 0; cx < cur->width; ++cx) {
            int px = x + cx - cur->hotspot_x;
            int py = y + cy - cur->hotspot_y;
            if (px < 0 || py < 0 || px >= fb_w || py >= fb_h) continue;
            uint32_t c = cur->pixels[cy * cur->width + cx];
            uint8_t a = (c >> 24) & 0xFF;
            if (a == 0) continue;
            fb[py * fb_w + px] = c;
        }
    }
}
