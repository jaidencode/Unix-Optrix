#include <stdint.h>
#include "fabric_defs.h"

// Only here: real variables
volatile int mouse_x = WIDTH / 2, mouse_y = HEIGHT / 2;
static int mouse_inited = 0;

void mouse_init(void) {
    // Initialize PS/2 mouse hardware here.
    mouse_inited = 1;
}

// Mouse packet handler (to be called from your IRQ code)
void mouse_handle_packet(uint8_t b1, uint8_t b2, uint8_t b3) {
    if (!mouse_inited) return;
    int dx = (b2 & 0xFF);  // signed movement
    int dy = (b3 & 0xFF);
    if (b1 & 0x10) dx |= 0xFFFFFF00; // sign extension for 9th bit (if needed)
    if (b1 & 0x20) dy |= 0xFFFFFF00;
    mouse_x += dx;
    mouse_y -= dy;
    // Clamp to screen bounds
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x >= WIDTH) mouse_x = WIDTH - 1;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_y >= HEIGHT) mouse_y = HEIGHT - 1;
}

int mouse_is_inited(void) { return mouse_inited; }
