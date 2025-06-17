#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "fs.h"
#include "container.h"
#include "taskbar.h"
#include "terminal.h"

#define DESKTOP_BG_COLOR 0x17

static window_t *test_win = 0;

static void draw_wallpaper(void) {
    for(int y=0; y<SCREEN_HEIGHT; y++)
        for(int x=0; x<SCREEN_WIDTH; x++)
            put_pixel(x, y, DESKTOP_BG_COLOR);
}

void desktop_init(void) {
    fs_init();
    taskbar_init();
    draw_wallpaper();
    container_init();
    test_win = container_create(96, 72, 512, 336, "Test Window", 0x02, 0x00);
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
    }
}

void desktop_redraw_region(int x, int y, int w, int h) {
    (void)x; (void)y; (void)w; (void)h;
    draw_wallpaper();
    container_draw();
}
