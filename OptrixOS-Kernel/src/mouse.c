#include "mouse.h"
#include "graphics.h"
#include "screen.h"
#include "ports.h"
#include <stdint.h>

static int mx = SCREEN_WIDTH/2;
static int my = SCREEN_HEIGHT/2;
static int clicked = 0;
static int mouse_present = 0;
static int cursor_visible = 1;
static int prev_x = -1, prev_y = -1;
static uint8_t saved_bg[4][4];

void mouse_set_visible(int vis) { cursor_visible = vis; }
int mouse_get_visible(void) { return cursor_visible; }

void mouse_draw(uint8_t bg_color) {
    (void)bg_color;
    if(prev_x != -1 && prev_y != -1 && cursor_visible) {
        for(int dy=0; dy<4; dy++)
            for(int dx=0; dx<4; dx++)
                put_pixel(prev_x-2+dx, prev_y-2+dy, saved_bg[dy][dx]);
        graphics_flush(prev_x-2, prev_y-2, 4, 4);
    }
    if(cursor_visible) {
        for(int dy=0; dy<4; dy++)
            for(int dx=0; dx<4; dx++)
                saved_bg[dy][dx] = get_pixel(mx-2+dx, my-2+dy);
        draw_rect(mx-2, my-2, 4, 4, 0x0F);
        graphics_flush(mx-2, my-2, 4, 4);
        prev_x = mx; prev_y = my;
    } else {
        prev_x = prev_y = -1;
    }
}

static void mouse_wait_input(void) {
    for(int i=0;i<100000;i++) if(inb(0x64) & 1) return;
}

static void mouse_wait_output(void) {
    for(int i=0;i<100000;i++) if(!(inb(0x64) & 2)) return;
}

static void mouse_write(uint8_t a) {
    mouse_wait_output();
    outb(0x64, 0xD4);
    mouse_wait_output();
    outb(0x60, a);
}

static uint8_t mouse_read(void) {
    mouse_wait_input();
    return inb(0x60);
}

void mouse_init(void) {
    mx = SCREEN_WIDTH/2;
    my = SCREEN_HEIGHT/2;
    clicked = 0;
    mouse_present = 0;

    /* Attempt to enable PS/2 mouse */
    mouse_wait_output();
    outb(0x64, 0xA8);            /* enable auxiliary device */
    mouse_write(0xFF);           /* reset */
    if(mouse_read() != 0xFA) { return; }
    if(mouse_read() != 0xAA) { return; }
    (void)mouse_read();          /* discard device ID */
    mouse_write(0xF4);           /* enable streaming */
    if(mouse_read() != 0xFA) { return; }

    mouse_present = 1;
}

int mouse_is_present(void) { return mouse_present; }

void mouse_update(void) {


    static uint8_t packet[3];
    static int cycle = 0;
    while(inb(0x64) & 1) {
        uint8_t status = inb(0x64);
        if(!(status & 0x20))
            break; /* data is from keyboard */
        packet[cycle++] = inb(0x60);
        if(cycle == 3) {
            int dx = (int8_t)packet[1];
            int dy = -(int8_t)packet[2];
            mx += dx;
            my += dy;
            clicked = packet[0] & 1;
            if(mx < 0) mx = 0;
            if(my < 0) my = 0;
            if(mx >= SCREEN_WIDTH) mx = SCREEN_WIDTH-1;
            if(my >= SCREEN_HEIGHT) my = SCREEN_HEIGHT-1;
            cycle = 0;
        }
    }
}

int mouse_get_x(void) { return mx; }
int mouse_get_y(void) { return my; }
int mouse_clicked(void) { return clicked; }
