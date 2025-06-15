#include "debug.h"
#include "serial.h"
#include "pmm.h"
#include "shell.h"
#include "keyboard.h"

void kernel_main(void) {
    serial_init();
    keyboard_init();
    debug_clear(0);
    debug_log("Kernel start");
    pmm_init();
    debug_log("Launching shell");
    shell_run();
    while (1) __asm__ __volatile__("hlt");
}
