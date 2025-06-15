#include "debug.h"
#include "serial.h"
#include "video.h"

static int debug_line = 0;

static void draw_char(int x, int y, char c, uint32_t color) {
    for (int row = 0; row < 8; ++row)
        for (int col = 0; col < 8; ++col)
            framebuffer[(y + row) * framebuffer_width + (x + col)] =
                (c == ' ' ? color : color | ((col ^ row) & 1));
}

static void draw_text(int x, int y, const char* s, uint32_t color) {
    for (int i = 0; s[i]; ++i)
        draw_char(x + i * 8, y, s[i], color);
}

void debug_clear(uint32_t color) {
    if (!framebuffer) return;
    for (uint32_t i = 0; i < framebuffer_width * framebuffer_height; ++i)
        framebuffer[i] = color;
    debug_line = 0;
}

void debug_log(const char* msg) {
    serial_write(msg);
    serial_write("\n");
    if (!framebuffer) return;
    draw_text(0, debug_line * 8, msg, 0xFFFFFFFF);
    debug_line++;
}
