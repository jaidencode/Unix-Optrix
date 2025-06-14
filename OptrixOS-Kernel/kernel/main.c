#include "vga.h"
#include "serial.h"
#include "pmm.h"
#include "fabric.h"   // <-- declare fabric_main()

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

    // Launch the GUI shell
    fabric_main();

    #define VGA_ADDR ((volatile char*)0xB8000)
    void vga_putc(int col, int row, char c, char color) {
        VGA_ADDR[2*(row*80 + col)] = c;
        VGA_ADDR[2*(row*80 + col)+1] = color;
    }

    // In kernel_main():
    vga_putc(0, 0, 'X', 0x0F);
    while(1) __asm__ __volatile__("hlt");
}
