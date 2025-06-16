#include "window.h"
#include "graphics.h"
#include "screen.h"
#include "mouse.h"

static int drag = 0;
static int resize = 0;
static int prev_mx, prev_my;
static int saved_x, saved_y, saved_w, saved_h;

void window_init(window_t *win, int x, int y, int w, int h,
                 const char *title, uint8_t color, uint8_t bg_color) {
    win->x = x; win->y = y; win->w = w; win->h = h;
    win->px = x; win->py = y; win->pw = w; win->ph = h;
    win->visible = 1;
    win->state = 0;
    win->color = color;
    win->bg_color = bg_color;
    win->title = title;
}

static void draw_buttons(int x, int y) {
    /* close */
    draw_rect(x-36, y+2, 10, 10, 0x04);
    /* minimize */
    draw_rect(x-24, y+2, 10, 10, 0x02);
    /* maximize */
    draw_rect(x-12, y+2, 10, 10, 0x03);
}

void window_draw(window_t* win) {
    if(!win || !win->visible) return;
    int x = win->x; int y = win->y; int w = win->w; int h = win->h;
    if(win->state == 1) { x = 0; y = 0; w = SCREEN_WIDTH; h = SCREEN_HEIGHT; }
    if(win->state == 2) return; /* minimized */

    /* clear previous location if window moved */
    if(x != win->px || y != win->py || w != win->pw || h != win->ph) {
        draw_rect(win->px, win->py, win->pw, win->ph, win->bg_color);
        win->px = x; win->py = y; win->pw = w; win->ph = h;
    }

    int show_bar = !(win->state == 1 && mouse_get_y() > 2);
    draw_rect(x, y, w, h, win->color);
    if(show_bar) {
        draw_rect(x, y, w, 14, 0x01); /* title bar */
        if(win->title) {
            const char *t = win->title;
            for(int i=0; t[i] && i<20; i++)
                screen_put_char((x+4-OFFSET_X)/CHAR_WIDTH + i,
                                (y+3-OFFSET_Y)/CHAR_HEIGHT, t[i], 0x0F);
        }
        draw_buttons(x+w, y);
    }
}

void window_handle_mouse(window_t *win, int mx, int my, int click) {
    if(!win || !win->visible) return;

    int x = win->x; int y = win->y; int w = win->w; int h = win->h;
    if(win->state == 1) { x = 0; y = 0; w = SCREEN_WIDTH; h = SCREEN_HEIGHT; }
    int show_bar = !(win->state == 1 && mouse_get_y() > 2);

    if(click && !drag && !resize) {
        if(show_bar && my >= y && my < y+14 && mx >= x+w-36 && mx < x+w-26) {
            win->visible = 0; /* close */
            return;
        } else if(show_bar && my >= y && my < y+14 && mx >= x+w-24 && mx < x+w-14) {
            win->state = 2; /* minimize */
            return;
        } else if(show_bar && my >= y && my < y+14 && mx >= x+w-12 && mx < x+w) {
            if(win->state == 1) {
                win->state = 0;
                win->x = saved_x; win->y = saved_y; win->w = saved_w; win->h = saved_h;
            } else {
                saved_x = win->x; saved_y = win->y; saved_w = win->w; saved_h = win->h;
                win->state = 1;
            }
            return;
        } else if(show_bar && my >= y && my < y+14) {
            drag = 1; prev_mx = mx; prev_my = my;
        } else if(mx >= x+w-4 && my >= y+h-4) {
            resize = 1; prev_mx = mx; prev_my = my;
        }
    } else if(click && drag) {
        win->x += mx - prev_mx; win->y += my - prev_my;
        prev_mx = mx; prev_my = my;
    } else if(click && resize) {
        win->w += mx - prev_mx; win->h += my - prev_my;
        prev_mx = mx; prev_my = my;
    } else {
        drag = resize = 0;
    }
}
