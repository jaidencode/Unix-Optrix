#include "container.h"
#include "taskbar.h"

static window_t windows[MAX_WINDOWS];
static int window_count = 0;

void container_init(void) {
    window_count = 0;
}

window_t* container_create(int x, int y, int w, int h, const char *title,
                           uint8_t color, uint8_t bg_color) {
    if(window_count >= MAX_WINDOWS)
        return 0;
    window_t *win = &windows[window_count++];
    window_init(win, x, y, w, h, title, color, bg_color);
    taskbar_register(win);
    return win;
}

void container_draw(void) {
    for(int i=0; i<window_count; i++)
        window_draw(&windows[i]);
}

void container_handle_mouse(int mx, int my, int click) {
    for(int i=window_count-1; i>=0; i--) {
        window_handle_mouse(&windows[i], mx, my, click);
        if(windows[i].closed) {
            taskbar_unregister(&windows[i]);
            for(int j=i; j<window_count-1; j++)
                windows[j] = windows[j+1];
            window_count--;
        }
    }
}
