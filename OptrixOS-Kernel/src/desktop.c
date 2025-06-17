#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "fs.h"

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

    /* basic colour approximations */
    const uint8_t frame      = 0x08; /* charcoal border */
    const uint8_t highlight  = 0x0F; /* bright white */
    const uint8_t shadow     = 0x07; /* mid grey */
    const uint8_t title_col1 = 0x0E; /* light gradient start */
    const uint8_t title_col2 = 0x0D; /* light gradient end */
    const uint8_t client_bg  = 0x0F; /* white */

    /* outer border */
    draw_rect(x, y, w, 1, frame);                    /* top    */
    draw_rect(x, y, 1, h, frame);                    /* left   */
    draw_rect(x, y+h-1, w, 1, frame);                /* bottom */
    draw_rect(x+w-1, y, 1, h, frame);                /* right  */

    /* highlight/shadow lines for bevel */
    draw_rect(x+1, y+1, w-2, 1, highlight);          /* top    */
    draw_rect(x+1, y+1, 1, h-2, highlight);          /* left   */
    draw_rect(x+1, y+h-2, w-2, 1, shadow);           /* bottom */
    draw_rect(x+w-2, y+1, 1, h-2, shadow);           /* right  */

    /* title bar (28px high including rule) */
    for(int i=0; i<27; i++) {
        uint8_t c = (i < 14) ? title_col1 : title_col2;
        draw_rect(x+1, y+1+i, w-2, 1, c);
    }
    draw_rect(x+1, y+27, w-2, 1, shadow); /* separator */

    /* client background */
    draw_rect(x+1, y+28, w-2, h-29, client_bg);

    /* window title */
    const char *title = "Test Window";
    for(int c=0; title[c]; c++)
        screen_put_char_offset(c, 0, title[c], frame, x+12, y+10);

    /* control icons */
    int btn_y = y + 6;
    int bx = x + w - 64;             /* 3 buttons + spacing + margin */
    draw_rect(bx,      btn_y, 16, 16, frame);  /* minimize */
    draw_rect(bx + 20, btn_y, 16, 16, frame);  /* maximize */
    draw_rect(bx + 40, btn_y, 16, 16, frame);  /* close    */
}

void desktop_init(void) {
    fs_init();
    draw_wallpaper();
    draw_demo_window();
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
    draw_demo_window();
}
