#include "screen.h"
#include "graphics.h"

void screen_init(void) {
    draw_rect(0, 0, 320, 200, 0); // clear to black
    draw_rect(0, 0, 320, 1, 15);   // top
    draw_rect(0, 199, 320, 1, 15); // bottom
    draw_rect(0, 0, 1, 200, 15);   // left
    draw_rect(319, 0, 1, 200, 15); // right
}
