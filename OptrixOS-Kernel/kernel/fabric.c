#include <stdint.h>
#include <stddef.h>
#include "cursor.h"

// --- KERNEL MEMORY STUBS FOR stb_image ---
void* kernel_malloc(size_t size) {
    extern void* pmm_alloc(void);  // Plug in your real allocator
    return pmm_alloc();
}
void kernel_free(void* ptr) { /* TODO: implement free if needed */ }
void* kernel_realloc(void* ptr, size_t size) { return NULL; }

#define STBI_MALLOC(sz)      kernel_malloc(sz)
#define STBI_REALLOC(p,sz)   kernel_realloc(p,sz)
#define STBI_FREE(p)         kernel_free(p)
#define STBI_NO_STDIO
#define STBI_NO_LIMITS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// --- UI CONSTANTS ---
#define WIDTH   1024
#define HEIGHT  768
#define FB_ADDR ((uint32_t*)0xE0000000) // Set to your real framebuffer address

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

// --- Load wallpaper from JPEG on ISO ---
static void load_wallpaper(void) {
    extern uint8_t _binary_OptrixOS_Kernel_resources_images_wallpaper_jpg_start[];
    extern uint8_t _binary_OptrixOS_Kernel_resources_images_wallpaper_jpg_end[];

    // Always use the embedded wallpaper to avoid ISO lookup issues
    size_t jpg_size = _binary_OptrixOS_Kernel_resources_images_wallpaper_jpg_end -
                      _binary_OptrixOS_Kernel_resources_images_wallpaper_jpg_start;
    uint8_t* jpg_data = _binary_OptrixOS_Kernel_resources_images_wallpaper_jpg_start;

    int w = 0, h = 0, comp = 0;
    if (jpg_data) {
        unsigned char* pixels = stbi_load_from_memory(jpg_data, jpg_size, &w, &h, &comp, 4);
        if (pixels && w == WIDTH && h == HEIGHT) {
            for (int i = 0; i < WIDTH * HEIGHT; ++i)
                wallpaper[i] = ((uint32_t*)pixels)[i];
        } else {
            for (int i = 0; i < WIDTH * HEIGHT; ++i)
                wallpaper[i] = 0xFF2266CC;
        }
        if (pixels)
            stbi_image_free(pixels);
        // free(jpg_data); // Only if malloc'd!
    } else {
        for (int i = 0; i < WIDTH * HEIGHT; ++i)
            wallpaper[i] = 0xFF2266CC;
    }
}

// --- Fabric main UI loop ---
void fabric_main(void) {
    if (!mouse_driver_loaded) {
        mouse_init();
        mouse_driver_loaded = 1;
    }

    // 1. Load wallpaper image ONCE
    load_wallpaper();

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

    while (1) {
        draw_wallpaper();         
        draw_window();
        draw_title_bar();
        // ...other UI...

        if (mouse_driver_loaded && system_cursor)
            draw_cursor(FB_ADDR, WIDTH, HEIGHT, system_cursor, mouse_x, mouse_y);

        __asm__ __volatile__("hlt");
    }
}
