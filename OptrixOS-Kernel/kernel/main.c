#include "vga.h"
#include "serial.h"
#include "pmm.h"
void kernel_main(void) {
    vga_clear(0x1);
    vga_print_center("Welcome to OptrixOS Minimum!", 0x0F);
    serial_init();
    serial_write("Serial initialized.\n");
    pmm_init();
    void* test = pmm_alloc();
    if (test)
        vga_print_at(0, 180, "Physical memory allocation succeeded.", 0x0A);
    else
        vga_print_at(0, 180, "Physical memory allocation failed!", 0x0C);
    while (1);
}
