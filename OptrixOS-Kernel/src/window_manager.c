#include "window_manager.h"
#include "taskbar.h"

#define MAX_WINDOWS 16

static window_t *windows[MAX_WINDOWS];
static int window_count = 0;

void window_manager_init(void) {
    window_count = 0;
}

void window_manager_add(window_t *win) {
    if(window_count < MAX_WINDOWS) {
        windows[window_count++] = win;
        taskbar_register(win);
    }
}

void window_manager_draw(void) {
    for(int i=0;i<window_count;i++)
        window_draw(windows[i]);
}

void window_manager_handle_mouse(int mx, int my, int click) {
    for(int i=window_count-1;i>=0;i--) {
        window_handle_mouse(windows[i], mx, my, click);
        if(windows[i]->closed) {
            taskbar_unregister(windows[i]);
            for(int j=i;j<window_count-1;j++)
                windows[j] = windows[j+1];
            window_count--;
        }
    }
}
