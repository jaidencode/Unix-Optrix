#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "fs.h"

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

static void draw_group_icon(int x, int y, const char *label) {
    draw_rect(x, y, 48, 48, DESKTOP_BG_COLOR);
    draw_rect(x+8, y+8, 20, 16, 0x0F);
    draw_rect(x+12, y+12, 20, 16, 0x07);
    put_pixel(x+20, y+14, 0x04);
    put_pixel(x+22, y+14, 0x02);
    put_pixel(x+24, y+14, 0x01);
    int w = str_len(label) * CHAR_WIDTH;
    draw_text(x + (48 - w)/2, y+32, label, TEXT_COLOR);
}

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

    /* fill client background */
    draw_rect(x+3, y+3, w-6, h-6, mid);

    /* title bar */
    const int title_h = 19;
    draw_rect(x+3, y+3, w-6, title_h, mid);

    /* program icon */
    int icon_x = x + 9;
    int icon_y = y + 6;
    draw_rect(icon_x-1, icon_y-1, 13, 13, TEXT_COLOR);
    draw_rect(icon_x, icon_y, 6, 6, 0x04);
    draw_rect(icon_x+7, icon_y, 6, 6, 0x02);
    draw_rect(icon_x, icon_y+7, 6, 6, 0x01);
    draw_rect(icon_x+7, icon_y+7, 6, 6, 0x0E);

    /* window title */
    const char *title = "Program Manager";
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

    /* client area below menu */
    int div2_y = menu_y + menu_h;
    draw_rect(x+3, div2_y, w-6, 1, sh);
    int client_y = div2_y + 1;
    int client_h = h - (client_y - y) - 3;
    draw_rect(x+3, client_y, w-6, client_h, mid);

    /* dialog box */
    int dlg_w = 316, dlg_h = 162;
    int dlg_x = x + (w - dlg_w)/2;
    int dlg_y = client_y + (client_h - dlg_h)/2;
    draw_bevel_rect(dlg_x, dlg_y, dlg_w, dlg_h, hl, sh, 0x0F);
    draw_rect(dlg_x+2, dlg_y+2, dlg_w-4, 9, 0x01);
    draw_rect(dlg_x+2, dlg_y+2, dlg_w-4, 1, hl);
    int logo_x = dlg_x + (dlg_w-16)/2;
    int logo_y = dlg_y+4;
    draw_rect(logo_x, logo_y, 7,7,0x04);
    draw_rect(logo_x+8, logo_y, 7,7,0x02);
    draw_rect(logo_x, logo_y+8,7,7,0x01);
    draw_rect(logo_x+8, logo_y+8,7,7,0x0E);
    draw_text(dlg_x + 10, dlg_y + 28,
              "386 Enhanced Mode Windows Version 3.10", TEXT_COLOR);
    draw_text(dlg_x + 10, dlg_y + 40,
              "Copyright (c) 1991, Microsoft Corp.", TEXT_COLOR);
    int ok_w = 60, ok_h = 18;
    int ok_x = dlg_x + (dlg_w - ok_w)/2;
    int ok_y = dlg_y + dlg_h - ok_h - 10;
    draw_bevel_rect(ok_x, ok_y, ok_w, ok_h, hl, sh, mid);
    draw_text(ok_x + (ok_w - 2*CHAR_WIDTH)/2, ok_y + 4, "OK", TEXT_COLOR);

    /* program group icons */
    const char *names[6] = {
        "Accessories","Games","StartUp",
        "Applications","Main","Office"
    };
    int groups_y = client_y + client_h - 60;
    for(int i=0;i<6;i++) {
        int gx = x + 16 + i*(48+32);
        draw_group_icon(gx, groups_y, names[i]);
    }
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
