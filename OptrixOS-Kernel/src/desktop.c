#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "terminal.h"
#include "window.h"

static int icon_x = 50;
static int icon_y = 50;
static int dragging = 0;
static int click_timer = 0;

static void draw_icon(void) {
    draw_rect(icon_x, icon_y, 32, 32, 0x07);
    screen_put_char((icon_x+12-OFFSET_X)/CHAR_WIDTH, (icon_y+12-OFFSET_Y)/CHAR_HEIGHT, 'T', 0x0F);
}

static int in_icon(int x, int y) {
    return (x >= icon_x && x < icon_x+32 && y >= icon_y && y < icon_y+32);
}

void desktop_init(void) {
    draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,0x17);
    icon_x = 50;
    icon_y = 50;
    draw_icon();
}

void desktop_run(void) {
    mouse_init();
    while(1) {
        mouse_update();
        int mx = mouse_get_x();
        int my = mouse_get_y();
        if(mouse_clicked()) {
            if(in_icon(mx,my)) {
                if(click_timer > 0 && click_timer < 20) {
                    terminal_init();
                    terminal_run();
                    desktop_init();
                } else {
                    dragging = 1;
                    click_timer = 1;
                }
            } else {
                click_timer = 1;
            }
        } else {
            if(dragging) {
                icon_x = mx-16;
                icon_y = my-16;
            }
            if(click_timer>0) click_timer++;
            if(click_timer>30) click_timer=0;
            dragging = 0;
        }
        desktop_init();
        draw_icon();
        draw_rect(mx-2,my-2,4,4,0x0F);
    }
}
