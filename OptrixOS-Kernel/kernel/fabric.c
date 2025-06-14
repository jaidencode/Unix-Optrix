// fabric.c : Minimal "Fabric" UI Shell
#include <stdint.h>

// If you use a boot info struct, fill these with the real values at boot!
#define FB_ADDR   ((uint32_t*)0xE0000000) // Replace this with the actual framebuffer address!
#define WIDTH     1024
#define HEIGHT    768

// Simple utility to fill the screen with a color
static void fill_screen(uint32_t color) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            FB_ADDR[y * WIDTH + x] = color;
        }
    }
}

// Draw a white rectangle in the center (the "window" of the shell)
static void draw_window() {
    int win_w = 400, win_h = 300;
    int x0 = (WIDTH  - win_w) / 2;
    int y0 = (HEIGHT - win_h) / 2;
    for (int y = 0; y < win_h; ++y) {
        for (int x = 0; x < win_w; ++x) {
            FB_ADDR[(y0 + y) * WIDTH + (x0 + x)] = 0xFFFFFFFF; // White
        }
    }
}

// Draw a simple "FABRIC" title bar
static void draw_title_bar() {
    int win_w = 400, win_h = 30;
    int x0 = (WIDTH  - win_w) / 2;
    int y0 = (HEIGHT - 300) / 2;
    for (int y = 0; y < win_h; ++y) {
        for (int x = 0; x < win_w; ++x) {
            FB_ADDR[(y0 + y) * WIDTH + (x0 + x)] = 0xFF2222CC; // Deep blue
        }
    }
    // NOTE: For true text, you'd want to blit a bitmap font; for demo, color only
}

// Main "Fabric" function, called from kernel_main
void fabric_main(void) {
    fill_screen(0xFF2266CC);  // Blue background
    draw_window();
    draw_title_bar();
    // Infinite loop
    while (1) { __asm__ __volatile__("hlt"); }
}
