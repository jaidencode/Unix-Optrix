#include <stdint.h>
#include <stddef.h>
#include "cursor.h"
#include "wallpaper.h"
#include "video.h"
#include "fabric_defs.h"

// --- UI CONSTANTS ---
#define FB_ADDR framebuffer

// Mouse globals -- ONLY declare here, define in mouse.c!
extern volatile int mouse_x, mouse_y;
extern void mouse_init(void); // Your mouse driver init

// System cursor pointer
static Cursor* system_cursor = NULL;
static int mouse_driver_loaded = 0;

// --- Wallpaper buffer ---
static uint32_t wallpaper[WIDTH * HEIGHT]; // fallback: holds wallpaper pixels

// --- Helper: Fill screen with a color ---
static void clear_screen(uint32_t color) {
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
        FB_ADDR[i] = color;
}

// --- Helper: Draw wallpaper image (assumes RGBA8888, 1024x768) ---
static void draw_wallpaper(void) {
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            FB_ADDR[y * WIDTH + x] = wallpaper[y * WIDTH + x];
}

// --- Draw window (demo) ---
static void draw_window(void) {
    int win_w = 400, win_h = 300;
    int x0 = (WIDTH - win_w) / 2;
    int y0 = (HEIGHT - win_h) / 2;
    for (int y = 0; y < win_h; ++y)
        for (int x = 0; x < win_w; ++x)
            FB_ADDR[(y0 + y) * WIDTH + (x0 + x)] = 0xFFFFFFFF;
}

// --- Draw blue title bar ---
static void draw_title_bar(void) {
    int win_w = 400, win_h = 30;
    int x0 = (WIDTH - win_w) / 2;
    int y0 = (HEIGHT - 300) / 2;
    for (int y = 0; y < win_h; ++y)
        for (int x = 0; x < win_w; ++x)
            FB_ADDR[(y0 + y) * WIDTH + (x0 + x)] = 0xFF2222CC;
}

// --- Simple text rendering on 32bpp framebuffer ---
static void fb_draw_char(int x, int y, char c, uint32_t color) {
    for (int row = 0; row < 8; ++row)
        for (int col = 0; col < 8; ++col)
            FB_ADDR[(y + row) * WIDTH + (x + col)] =
                (c == ' ' ? color : color);
}

static void fb_draw_text(int x, int y, const char* s, uint32_t color) {
    for (int i = 0; s[i]; ++i)
        fb_draw_char(x + i * 8, y, s[i], color);
}

// --- Load wallpaper from JPEG on ISO ---
static int desktop_found = 0;

static void load_wallpaper(void) {
    int w = 0, h = 0;
    uint32_t* pixels = load_wallpaper_from_iso(
        "OptrixOS-Kernel/resources/images/wallpaper.jpg", &w, &h);
    if (pixels && w == WIDTH && h == HEIGHT) {
        for (int i = 0; i < WIDTH * HEIGHT; ++i)
            wallpaper[i] = pixels[i];
        desktop_found = 1;
    } else {
        for (int i = 0; i < WIDTH * HEIGHT; ++i)
            wallpaper[i] = 0xFF2266CC;
        desktop_found = 0;
    }
    if (pixels)
        free_wallpaper(pixels);
}

// --- Fabric main UI loop ---
void fabric_main(void) {
    debug_log("fabric_main start");
    if (!mouse_driver_loaded) {
        mouse_init();
        mouse_driver_loaded = 1;
    }

    // 1. Load wallpaper image ONCE
    load_wallpaper();
    debug_log(desktop_found ? "wallpaper loaded" : "wallpaper missing");

    // 2. Load cursor file ONCE at startup (edit symbol name for your path!)
    if (!system_cursor) {
        extern uint8_t _binary_OptrixOS_Kernel_resources_cursors_oxy_chrome_pointing_hand_cur_start[];
        extern uint8_t _binary_OptrixOS_Kernel_resources_cursors_oxy_chrome_pointing_hand_cur_end[];
        size_t cursor_size = _binary_OptrixOS_Kernel_resources_cursors_oxy_chrome_pointing_hand_cur_end
                           - _binary_OptrixOS_Kernel_resources_cursors_oxy_chrome_pointing_hand_cur_start;
        system_cursor = load_cursor_from_cur(
            _binary_OptrixOS_Kernel_resources_cursors_oxy_chrome_pointing_hand_cur_start,
            cursor_size
        );
        // fallback: make your own cursor if needed
    }

    debug_log("fabric_main entering loop");
    while (1) {
        draw_wallpaper();
        draw_window();
        draw_title_bar();
        fb_draw_text(0, 0, desktop_found ? "Desktop Found: True" : "Desktop Found: False", 0xFFFFFFFF);
        // ...other UI...

        if (mouse_driver_loaded && system_cursor)
            draw_cursor(FB_ADDR, WIDTH, HEIGHT, system_cursor, mouse_x, mouse_y);

        __asm__ __volatile__("hlt");
    }
}
