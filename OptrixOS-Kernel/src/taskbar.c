#include "taskbar.h"
#include "graphics.h"
#include "screen.h"

#define MAX_TASKS 10
#define TASKBAR_COLOR 0x01

static window_t *tasks[MAX_TASKS];
static int task_count = 0;

void taskbar_init(void) {
    task_count = 0;
    taskbar_draw();
}

void taskbar_register(window_t *win) {
    if(task_count < MAX_TASKS) {
        tasks[task_count++] = win;
        taskbar_draw();
    }
}

void taskbar_unregister(window_t *win) {
    for(int i=0;i<task_count;i++) {
        if(tasks[i]==win) {
            for(int j=i;j<task_count-1;j++)
                tasks[j]=tasks[j+1];
            task_count--;
            break;
        }
    }
    taskbar_draw();
}

void taskbar_draw(void) {
    const int bar_h = 16;
    draw_rect(0, SCREEN_HEIGHT - bar_h, SCREEN_WIDTH, bar_h, TASKBAR_COLOR);
    int w = SCREEN_WIDTH / (task_count > 0 ? task_count : 1);
    for(int i=0;i<task_count;i++) {
        int x = i * w;
        draw_rect(x+1, SCREEN_HEIGHT - bar_h + 1, w-2, bar_h-2, 0x07);
        const char *t = tasks[i]->title;
        if(t) {
            for(int c=0;t[c] && c<10 && x + 8 + c*CHAR_WIDTH < x+w-4;c++)
                screen_put_char_offset((x+8-OFFSET_X)/CHAR_WIDTH + c,
                                       (SCREEN_HEIGHT - bar_h + 4 - OFFSET_Y)/CHAR_HEIGHT,
                                       t[c], 0x0F,
                                       0,0);
        }
    }
}

void taskbar_handle_click(int x, int y) {
    const int bar_h = 16;
    if(y < SCREEN_HEIGHT - bar_h)
        return;
    if(task_count == 0)
        return;
    int w = SCREEN_WIDTH / task_count;
    int idx = x / w;
    if(idx >= task_count)
        return;
    window_t *win = tasks[idx];
    if(win->state == 2) {
        win->state = 0;
        window_draw(win);
    } else {
        win->state = 2;
        window_draw(win);
    }
}
