#include "debug.h"
#include "serial.h"
#include "pmm.h"
#include "fabric.h"
#include "iso9660.h"

void kernel_main(void) {
    serial_init();
    debug_clear(0x000000);
    debug_log("Kernel start");

    debug_log("Initializing PMM");
    pmm_init();
    void* test = pmm_alloc();
    if (test)
        debug_log("Physical memory allocation succeeded");
    else
        debug_log("Physical memory allocation failed");

    debug_log("Testing ISO9660 read");
    size_t sz = 0;
    void* file = iso9660_load_file("OptrixOS-Kernel/resources/images/wallpaper.jpg", &sz);
    if (file && sz)
        debug_log("ISO file read success");
    else
        debug_log("ISO file read failed");

    debug_log("Launching GUI");
    fabric_main();

    while (1) __asm__ __volatile__("hlt");
}
