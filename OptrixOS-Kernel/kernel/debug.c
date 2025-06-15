#include "debug.h"
#include "serial.h"
#include "terminal.h"

static int debug_line = 0;

void debug_clear(uint32_t color) {
    (void)color;
    terminal_clear();
    debug_line = 0;
}

void debug_log(const char* msg) {
    serial_write(msg);
    serial_write("\n");
    terminal_write(msg);
    terminal_write("\n");
    debug_line++;
}
