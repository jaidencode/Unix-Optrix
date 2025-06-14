#include <stdint.h>
#include <stddef.h>
#include "cursor.h"    // Cursor struct + draw_cursor()
#include "iso9660.h"   // Your ISO9660 file loader
#include "vga.h"       // For fallback error message

#define WIDTH  1024
#define HEIGHT 768
#define CURSOR_PATH "RESOURCES/CURSORS/OXY-CHROME/POINTING_HAND.CUR"

// These should be updated by your PS/2 mouse IRQ handler:
extern volatile int mouse_x;
extern volatile int mouse_y;

#define FB_ADDR ((uint32_t*)0xE0000000) // Set to your framebuffer base

static Cursor* system_cursor = NULL;

static void clear_screen(uint32_t color) {
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            FB_ADDR[y * WIDTH + x] = color;
}

static void draw_window(void) {
    int win_w = 400, win_h = 300;
    int x0 = (WIDTH - win_w) / 2;
    int y0 = (HEIGHT - win_h) / 2;
    for (int y = 0; y < win_h; ++y)
        for (int x = 0; x < win_w; ++x)
            FB_ADDR[(y0 + y) * WIDTH + (x0 + x)] = 0xFFFFFFFF;
}

static void draw_title_bar(void) {
    int win_w = 400, win_h = 30;
    int x0 = (WIDTH - win_w) / 2;
    int y0 = (HEIGHT - 300) / 2;
    for (int y = 0; y < win_h; ++y)
        for (int x = 0; x < win_w; ++x)
            FB_ADDR[(y0 + y) * WIDTH + (x0 + x)] = 0xFF2222CC;
}

// Fabric main UI loop
void fabric_main(void) {
    // 1. Load cursor file from ISO9660 ONCE at startup
    if (!system_cursor) {
        size_t cursz = 0;
        void* curfile = iso9660_load_file(CURSOR_PATH, &cursz);
        if (curfile && cursz > 0) {
            system_cursor = load_cursor_from_cur((const uint8_t*)curfile, cursz);
        }
        if (!system_cursor) {
            vga_print_center("Cursor file missing! Using fallback.", 0x0C);
            // fallback: draw a square pointer, or set a minimal default
            // Optionally allocate a minimal cursor struct here
        }
    }

    // 2. Main UI "loop" — redraw everything each time, no cursor trails
    while (1) {
        clear_screen(0xFF2266CC); // blue desktop
        draw_window();
        draw_title_bar();
        // ...draw more UI if you wish...

        // Draw the mouse cursor LAST (on top, no trails)
        if (system_cursor)
            draw_cursor(FB_ADDR, WIDTH, HEIGHT, system_cursor, mouse_x, mouse_y);

        // Wait for next event, or just HLT for demo:
        __asm__ __volatile__("hlt");
    }
}
