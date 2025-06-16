#include "mouse.h"
#include "keyboard.h"
#include "graphics.h"
#include "screen.h"

static int mx = SCREEN_WIDTH/2;
static int my = SCREEN_HEIGHT/2;
static int clicked = 0;

void mouse_init(void) {
    mx = SCREEN_WIDTH/2;
    my = SCREEN_HEIGHT/2;
    clicked = 0;
}

void mouse_update(void) {
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
}

int mouse_get_x(void) { return mx; }
int mouse_get_y(void) { return my; }
int mouse_clicked(void) { return clicked; }
