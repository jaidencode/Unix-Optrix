#include <stdint.h>
#include <stdbool.h>
#include "fabric_defs.h"

volatile int mouse_x = WIDTH / 2;
volatile int mouse_y = HEIGHT / 2;
volatile bool mouse_ready = false;

void mouse_handle_packet(uint8_t b1, uint8_t b2, uint8_t b3) {
    // Parse PS/2 3-byte packet for standard mouse

    // Extract signed deltas
    int dx = (int8_t)b2; // X movement (signed)
    int dy = (int8_t)b3; // Y movement (signed; PS/2 Y is negative up)
    
    // Update position (invert dy for top-left origin)
    mouse_x += dx;
    mouse_y -= dy;

    // Clamp to screen bounds
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x >= WIDTH)  mouse_x = WIDTH - 1;
    if (mouse_y >= HEIGHT) mouse_y = HEIGHT - 1;
}
