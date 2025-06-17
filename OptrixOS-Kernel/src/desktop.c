#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "fs.h"
#include "window_manager.h"

#define TEXT_COLOR 0x00

#define DESKTOP_BG_COLOR 0x17

static int str_len(const char *s) {
    int n = 0;
    while(s[n]) n++;
    return n;
}

static void draw_text(int x, int y, const char *str, uint8_t color) {
    for(int i=0; str[i]; i++)
        screen_put_char_offset(i, 0, str[i], color, x, y);
}

static void draw_bevel_rect(int x, int y, int w, int h,
                            uint8_t hl, uint8_t sh, uint8_t fill) {
    draw_rect(x+2, y+2, w-4, h-4, fill);
    draw_rect(x, y, w, 2, hl);
    draw_rect(x, y, 2, h, hl);
    draw_rect(x, y+h-2, w, 2, sh);
    draw_rect(x+w-2, y, 2, h, sh);
}


static void draw_wallpaper(void) {
    for(int y=0; y<SCREEN_HEIGHT; y++)
        for(int x=0; x<SCREEN_WIDTH; x++)
            put_pixel(x, y, DESKTOP_BG_COLOR);
}

static void draw_simple_window(void) {
    const int w = 480;
    const int h = 360;
    const int x = (640 - w)/2;
    const int y = (480 - h)/2;
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

    /* fill client background */
    draw_rect(x+3, y+3, w-6, h-6, mid);

    /* title bar */
    const int title_h = 19;
    draw_rect(x+3, y+3, w-6, title_h, mid);

    /* icon placeholder */
    int icon_x = x + 9;
    int icon_y = y + 6;
    draw_rect(icon_x, icon_y, 13, 13, TEXT_COLOR);

    /* window title */
    const char *title = "Test UI";
    int title_w = str_len(title) * CHAR_WIDTH;
    draw_text(x + (w - title_w)/2, y + 7, title, TEXT_COLOR);

    /* control buttons */
    int btn_w = 15, btn_h = 13;
    int btn_y = y + 6;
    int min_x = x + w - 3 - btn_w*2 - 2;
    int max_x = x + w - 3 - btn_w;
    draw_bevel_rect(min_x, btn_y, btn_w, btn_h, hl, sh, mid);
    draw_bevel_rect(max_x, btn_y, btn_w, btn_h, hl, sh, mid);
    draw_rect(min_x+4, btn_y+btn_h/2+2, btn_w-8, 1, TEXT_COLOR); /* underscore */
    draw_rect(max_x+4, btn_y+4, btn_w-8, btn_h-8, TEXT_COLOR);   /* square */

    /* divider and menu bar */
    int div1_y = y + 3 + title_h;
    draw_rect(x+3, div1_y, w-6, 1, sh);
    int menu_h = 18;
    int menu_y = div1_y + 1;
    draw_rect(x+3, menu_y, w-6, menu_h, 0x0E);
    draw_text(x+10, menu_y+4, "File", TEXT_COLOR);
    draw_text(x+58, menu_y+4, "Options", TEXT_COLOR);
    draw_text(x+124, menu_y+4, "Window", TEXT_COLOR);
    draw_text(x+192, menu_y+4, "Help", TEXT_COLOR);
    draw_rect(x+3+176, menu_y+2, 1, menu_h-4, sh);

    /* client area */
    int div2_y = menu_y + menu_h;
    draw_rect(x+3, div2_y, w-6, 1, sh);
    int client_y = div2_y + 1;
    int client_h = h - (client_y - y) - 3;
    draw_rect(x+3, client_y, w-6, client_h, mid);
}

void desktop_init(void) {
    fs_init();
    draw_wallpaper();
    window_manager_init();
    draw_simple_window();
}

void desktop_run(void) {
    mouse_init();
    mouse_set_visible(1);
    mouse_draw(DESKTOP_BG_COLOR);
    while(1) {
        mouse_update();
        window_manager_handle_mouse(mouse_get_x(), mouse_get_y(), mouse_clicked());
        mouse_draw(DESKTOP_BG_COLOR);
    }
}

void desktop_redraw_region(int x, int y, int w, int h) {
    (void)x; (void)y; (void)w; (void)h;
    draw_wallpaper();
    draw_simple_window();
}
