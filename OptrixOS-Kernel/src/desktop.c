#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "fs.h"
#include "svg.h"
#include "windows31_svg.h"

#define DESKTOP_BG_COLOR 0x17

static void draw_wallpaper(void) {
    for(int y=0; y<SCREEN_HEIGHT; y++)
        for(int x=0; x<SCREEN_WIDTH; x++)
            put_pixel(x, y, DESKTOP_BG_COLOR);
}

static void draw_demo_window(void) {
    const int x = 68;
    const int y = 48;
    const int w = 512;
    const int h = 336;
    const uint8_t hl = 0x0F; /* highlight */
    const uint8_t sh = 0x08; /* shadow */
    const uint8_t mid = 0x07; /* medium grey */

    /* outer frame */
    draw_rect(x, y, w, 2, hl);
    draw_rect(x, y, 2, h, hl);
    draw_rect(x, y+h-2, w, 2, sh);
    draw_rect(x+w-2, y, 2, h, sh);

    /* inner ridge */
    draw_rect(x+2, y+2, w-4, 1, mid);
    draw_rect(x+2, y+2, 1, h-4, mid);
    draw_rect(x+3, y+h-3, w-4, 1, mid);
    draw_rect(x+w-3, y+2, 1, h-4, mid);

    /* client area */
    draw_rect(x+3, y+3, w-6, h-6, mid);
}

void desktop_init(void) {
    fs_init();
    draw_wallpaper();
    svg_render(windows31_svg);
}

void desktop_run(void) {
    mouse_init();
    mouse_set_visible(1);
    mouse_draw(DESKTOP_BG_COLOR);
    while(1) {
        mouse_update();
        mouse_draw(DESKTOP_BG_COLOR);
    }
}

void desktop_redraw_region(int x, int y, int w, int h) {
    (void)x; (void)y; (void)w; (void)h;
    draw_wallpaper();
    svg_render(windows31_svg);
}
