#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "fs.h"
#include "container.h"
#include "taskbar.h"
#include "terminal.h"
#include "mem.h"

#define WALL_COLOR1 0x1B /* Windows 95 teal */
#define WALL_COLOR2 0x13 /* darker teal */

static uint8_t *wallpaper_buf = 0;

static window_t *test_win = 0;

static void init_wallpaper(void) {
    wallpaper_buf = mem_alloc(SCREEN_WIDTH * SCREEN_HEIGHT);
    if(!wallpaper_buf) return;
    for(int y=0; y<SCREEN_HEIGHT; y++) {
        for(int x=0; x<SCREEN_WIDTH; x++) {
            uint8_t color = ((x/8 + y/8) % 2) ? WALL_COLOR1 : WALL_COLOR2;
            wallpaper_buf[y * SCREEN_WIDTH + x] = color;
        }
    }
}

static void draw_wallpaper(void) {
    if(!wallpaper_buf)
        init_wallpaper();
    if(wallpaper_buf) {
        for(int y=0; y<SCREEN_HEIGHT; y++) {
            for(int x=0; x<SCREEN_WIDTH; x++) {
                put_pixel(x, y, wallpaper_buf[y * SCREEN_WIDTH + x]);
            }
        }
    }
}

void desktop_init(void) {
    fs_init();
    taskbar_init();
    uint8_t *buf = mem_alloc(SCREEN_WIDTH * SCREEN_HEIGHT);
    if(buf)
        graphics_set_backbuffer(buf);
    draw_wallpaper();
    graphics_present();
    container_init();
    test_win = container_create(96, 72, 512, 336, "Test Window", 0x07, 0x00);
    terminal_set_window(test_win);
}

void desktop_run(void) {
    mouse_init();
    mouse_set_visible(1);
    while(1) {
        mouse_update();
        mouse_clear();
        container_handle_mouse(mouse_get_x(), mouse_get_y(), mouse_clicked());
        container_draw();
        mouse_draw();
        graphics_present();
    }
}

void desktop_redraw_region(int x, int y, int w, int h) {
    if(wallpaper_buf) {
        if(x < 0) { w += x; x = 0; }
        if(y < 0) { h += y; y = 0; }
        if(x + w > SCREEN_WIDTH)  w = SCREEN_WIDTH - x;
        if(y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
        for(int yy=y; yy<y+h; yy++) {
            for(int xx=x; xx<x+w; xx++) {
                put_pixel(xx, yy, wallpaper_buf[yy * SCREEN_WIDTH + xx]);
            }
        }
    } else {
        draw_wallpaper();
    }
    container_draw();
}
