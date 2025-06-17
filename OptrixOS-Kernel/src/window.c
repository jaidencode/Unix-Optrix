#include "window.h"
#include "graphics.h"
#include "screen.h"
#include "mouse.h"
#include "taskbar.h"
#include "terminal.h"
#include "desktop.h"

static int drag = 0;
static int resize = 0;
static int prev_mx, prev_my;
static int saved_x, saved_y, saved_w, saved_h;

void window_init(window_t *win, int x, int y, int w, int h,
                 const char *title, uint8_t color, uint8_t bg_color) {
    win->x = x; win->y = y; win->w = w; win->h = h;
    win->px = -1; win->py = -1; win->pw = -1; win->ph = -1;
    win->visible = 1;
    win->state = 0;
    win->closed = 0;
    win->color = color;
    win->bg_color = bg_color;
    win->title = title;
}

void window_close(window_t *win) {
    if(!win || !win->visible) return;
    desktop_redraw_region(win->x, win->y, win->w, win->h);
    win->visible = 0;
    win->closed = 1;
    win->px = win->py = -1;
    taskbar_unregister(win);
}

static void draw_button(int bx, int by) {
    const uint8_t base = 0x07;
    const uint8_t highlight = 0x0F;
    const uint8_t shadow = 0x08;
    draw_rect(bx, by, 8, 8, base);
    draw_rect(bx, by, 8, 1, highlight);
    draw_rect(bx, by, 1, 8, highlight);
    draw_rect(bx, by+7, 8, 1, shadow);
    draw_rect(bx+7, by, 1, 8, shadow);
}

static void draw_buttons(int x, int y) {
    draw_button(x-40, y+4); /* close */
    draw_button(x-26, y+4); /* minimize */
    draw_button(x-12, y+4); /* maximize */
}

void window_draw(window_t* win) {
    if(!win || !win->visible) return;
    int x = win->x; int y = win->y; int w = win->w; int h = win->h;
    if(win->state == 1) { x = 0; y = 0; w = SCREEN_WIDTH; h = SCREEN_HEIGHT; }
    if(win->state == 2) return; /* minimized */

    /* clear previous location if window moved */
    if(x != win->px || y != win->py || w != win->pw || h != win->ph) {
        if(win->px >= 0 && win->py >= 0)
            desktop_redraw_region(win->px, win->py, win->pw, win->ph);
        win->px = x; win->py = y; win->pw = w; win->ph = h;
    } else {
        /* nothing changed - avoid redraw flicker */
        return;
    }

    int bar_h = 20;
    const uint8_t border = 0x07;   /* grey */
    const uint8_t highlight = 0x0F;/* white */
    const uint8_t shadow = 0x08;   /* dark grey */

    /* outer frame */
    draw_rect(x, y, w, h, border);
    draw_rect(x, y, w, 1, highlight);          /* top highlight */
    draw_rect(x, y, 1, h, highlight);          /* left highlight */
    draw_rect(x, y+h-1, w, 1, shadow);         /* bottom shadow */
    draw_rect(x+w-1, y, 1, h, shadow);         /* right shadow */

    /* title bar */
    draw_rect(x+2, y+2, w-4, bar_h-2, border);
    draw_rect(x+2, y+2, w-4, 1, highlight);
    draw_rect(x+2, y+bar_h-1, w-4, 1, shadow);

    /* client area */
    draw_rect(x+2, y+bar_h, w-4, h-bar_h-2, win->color);

    /* window title centered */
    if(win->title) {
        int len=0; while(win->title[len]) len++;
        int tx = x + (w - len*CHAR_WIDTH)/2;
        for(int c=0;c<len;c++)
            screen_put_char_offset(c,0,win->title[c],0x0F,tx, y+6);
    }

    draw_buttons(x+w-4, y+2);
}

void window_handle_mouse(window_t *win, int mx, int my, int click) {
    if(!win || !win->visible) return;

    int x = win->x; int y = win->y; int w = win->w; int h = win->h;
    if(win->state == 1) { x = 0; y = 0; w = SCREEN_WIDTH; h = SCREEN_HEIGHT; }
    int show_bar = 1;
    int bar_h = 20;

    if(click && !drag && !resize) {
        if(show_bar && my >= y && my < y+bar_h-2 && mx >= x+w-40 && mx < x+w-32) {
            window_close(win);
            return;
        } else if(show_bar && my >= y && my < y+bar_h-2 && mx >= x+w-26 && mx < x+w-18) {
            win->state = 2; /* minimize */
            return;
        } else if(show_bar && my >= y && my < y+bar_h-2 && mx >= x+w-12 && mx < x+w-4) {
            if(win->state == 1) {
                win->state = 0;
                win->x = saved_x; win->y = saved_y; win->w = saved_w; win->h = saved_h;
            } else {
                saved_x = win->x; saved_y = win->y; saved_w = win->w; saved_h = win->h;
                win->state = 1;
            }
            return;
        } else if(show_bar && my >= y && my < y+bar_h-2) {
            drag = 1; prev_mx = mx; prev_my = my;
        } else if(mx >= x+w-4 && my >= y+h-4) {
            resize = 1; prev_mx = mx; prev_my = my;
        }
    } else if(click && drag) {
        win->x += mx - prev_mx; win->y += my - prev_my;
        prev_mx = mx; prev_my = my;
        terminal_recursive_update();
    } else if(click && resize) {
        win->w += mx - prev_mx; win->h += my - prev_my;
        prev_mx = mx; prev_my = my;
        terminal_recursive_update();
    } else {
        drag = resize = 0;
    }
}
