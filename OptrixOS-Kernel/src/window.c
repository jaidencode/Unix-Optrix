#include "window.h"
#include "graphics.h"
#include "screen.h"
#include "mouse.h"
#include <stddef.h>

static int drag = 0;
static int resize = 0;
static int prev_mx, prev_my;
static int saved_x, saved_y, saved_w, saved_h;

static int menu_visible = 0;
static int menu_x, menu_y;
static window_t *menu_win = NULL;

void window_init(window_t *win, int x, int y, int w, int h,
                 const char *title, uint8_t color, uint8_t bg_color) {
    win->x = x; win->y = y; win->w = w; win->h = h;
    win->px = -1; win->py = -1; win->pw = -1; win->ph = -1;
    win->visible = 1;
    win->state = 0;
    win->color = color;
    win->bg_color = bg_color;
    win->title = title;
}

static void draw_buttons(int x, int y) {
    draw_rounded_rect(x-36, y+3, 6, 6, 3, 0x04); /* close */
    draw_rounded_rect(x-24, y+3, 6, 6, 3, 0x02); /* minimize */
    draw_rounded_rect(x-12, y+3, 6, 6, 3, 0x03); /* maximize */
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
    } else {
        /* nothing changed - avoid redraw flicker */
        return;
    }

    int show_bar = !(win->state == 1 && mouse_get_y() > 2);
    /* shadow */
    draw_rounded_rect(x+2, y+2, w, h, 6, win->bg_color);
    /* window body */
    draw_rounded_rect(x, y, w, h, 6, win->color);
    /* border */
    draw_rounded_rect(x, y, w, h, 6, 0x08);
    if(show_bar) {
        draw_rounded_rect(x, y, w, 14, 6, 0x01); /* title bar */
        if(win->title) {
            const char *t = win->title;
            for(int i=0; t[i] && i<20; i++)
                screen_put_char((x+4-OFFSET_X)/CHAR_WIDTH + i,
                                (y+3-OFFSET_Y)/CHAR_HEIGHT, t[i], 0x0F);
        }
        draw_buttons(x+w, y);
    }
    if(menu_visible && win == menu_win) {
        draw_rect(menu_x, menu_y, 80, 12, 0x01);
        const char *c = "Close";
        for(int i=0;c[i];i++)
            screen_put_char((menu_x+2-OFFSET_X)/CHAR_WIDTH + i,
                            (menu_y+2-OFFSET_Y)/CHAR_HEIGHT, c[i], 0x0F);
    }
}

void window_handle_mouse(window_t *win, int mx, int my, int click, int rclick) {
    if(!win || !win->visible) return;

    int x = win->x; int y = win->y; int w = win->w; int h = win->h;
    if(win->state == 1) { x = 0; y = 0; w = SCREEN_WIDTH; h = SCREEN_HEIGHT; }
    int show_bar = !(win->state == 1 && mouse_get_y() > 2);

    if(rclick && show_bar && my >= y && my < y+14) {
        menu_visible = 1;
        menu_x = mx;
        menu_y = my;
        menu_win = win;
        return;
    }
    if(click && menu_visible && win == menu_win &&
       mx >= menu_x && mx < menu_x+80 && my >= menu_y && my < menu_y+12) {
        draw_rect(win->x, win->y, win->w, win->h, win->bg_color);
        win->visible = 0;
        win->px = win->py = -1;
        menu_visible = 0;
        return;
    }
    if(click && !drag && !resize) {
        if(show_bar && my >= y && my < y+14 && mx >= x+w-36 && mx < x+w-26) {
            draw_rect(win->x, win->y, win->w, win->h, win->bg_color);
            win->visible = 0; /* close */
            win->px = win->py = -1;
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
        if(!rclick) menu_visible = 0;
    }
}
