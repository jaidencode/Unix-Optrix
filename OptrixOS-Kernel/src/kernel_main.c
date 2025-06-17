#include "screen.h"
#include "boot_logo.h"
#include "desktop.h"
#include "driver.h"
#include "cpu_driver.h"
#include "mem.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (2*1024*1024)

void kernel_main(void) {
    screen_init();
    boot_logo();
    mem_init(HEAP_BASE, HEAP_SIZE);
    cpu_driver_register();
    driver_init_all();
    desktop_init();
    desktop_run();
}
