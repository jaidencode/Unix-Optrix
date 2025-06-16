#include "mouse.h"
#include "keyboard.h"
#include "graphics.h"
#include "screen.h"
#include "ports.h"
#include <stdint.h>

static int mx = SCREEN_WIDTH/2;
static int my = SCREEN_HEIGHT/2;
static int clicked = 0;
static int mouse_present = 0;
static int fallback = 0;

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
    if(mouse_read() != 0xFA) { fallback = 1; return; }
    if(mouse_read() != 0xAA) { fallback = 1; return; }
    (void)mouse_read();          /* discard device ID */
    mouse_write(0xF4);           /* enable streaming */
    if(mouse_read() != 0xFA) { fallback = 1; return; }

    mouse_present = 1;
    fallback = 0;
}

int mouse_is_present(void) { return mouse_present; }

void mouse_update(void) {
    if(!mouse_present && fallback) {
        /* keyboard based fallback */
        char c = keyboard_getchar();
        clicked = 0;
        if(c == 'w') my -= 4;
        else if(c == 's') my += 4;
        else if(c == 'a') mx -= 4;
        else if(c == 'd') mx += 4;
        else if(c == 'm') clicked = 1;
        if(mx < 0) mx = 0;
        if(my < 0) my = 0;
        if(mx >= SCREEN_WIDTH) mx = SCREEN_WIDTH-1;
        if(my >= SCREEN_HEIGHT) my = SCREEN_HEIGHT-1;
        return;
    }

    static uint8_t packet[3];
    static int cycle = 0;
    while(inb(0x64) & 1) {
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
