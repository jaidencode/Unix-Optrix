#include "debug.h"
#include "serial.h"
#include "pmm.h"
#include "shell.h"
#include "keyboard.h"
#include "terminal.h"

void kernel_main(void) {
    serial_init();
    terminal_initialize();
    debug_clear(0);
    debug_log("[ OK ] Serial initialized");
    keyboard_init();
    debug_log("[ OK ] Keyboard detected");
    debug_log("[ OK ] Terminal ready");
    debug_log("[ .. ] Initializing memory");
    pmm_init();
    debug_log("[ OK ] Memory manager online");
    debug_log("Launching shell");
    shell_run();
    while (1) __asm__ __volatile__("hlt");
}
