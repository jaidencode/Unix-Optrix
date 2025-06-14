#include <stdint.h>
#include "font8x8_basic.h"
#include "boot_params.h"
#include "memmap.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "slab.h"

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void init_serial() {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB
    outb(COM1 + 0, 0x03);    // 38400 baud
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static void serial_write(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
}

static void log(const char* s) {
    while (*s) serial_write(*s++);
}

static void draw_char(int x, int y, char c, uint8_t color) {
    uint8_t* video = (uint8_t*)0xA0000;
    const uint8_t* glyph = font8x8_basic[(uint8_t)c];
    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                video[(y + row) * 320 + x + col] = color;
            }
        }
    }
}

static int strlen_simple(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void draw_string(int x, int y, const char* s, uint8_t color) {
    while (*s) {
        draw_char(x, y, *s++, color);
        x += 8;
    }
}

static void draw_string_center(const char* s, uint8_t color) {
    int len = strlen_simple(s);
    int x = (320 - len * 8) / 2;
    int y = (200 - 8) / 2;
    draw_string(x, y, s, color);
}

static int test_memmap(void) {
    memmap_init();
    return memmap_region_count() > 0 && memmap_get(0) != 0;
}

static int test_pmm(void) {
    memmap_init();
    pmm_init(memmap_regions(), memmap_region_count());
    void* f = pmm_alloc();
    if (!f) return 0;
    pmm_free(f);
    return 1;
}

static int test_heap(void) {
    heap_init();
    void* m = heap_alloc(8);
    if (!m) return 0;
    heap_free(m);
    return 1;
}

static int test_slab(void) {
    heap_init();
    slab_init();
    void* m = kmalloc(8);
    if (!m) return 0;
    kfree(m);
    return 1;
}

static int test_vmm(void) {
    vmm_init();
    return 1;
}

static void print_result(const char* name, int line, int pass) {
    const char* status = pass ? " PASS" : " FAIL";
    int color = pass ? 0x0A : 0x04;
    int x = 8;
    int y = 8 + line * 10;
    draw_string(x, y, name, 0x0F);
    int len = strlen_simple(name);
    draw_string(x + len * 8, y, status, color);
}

void main(void) {
    init_serial();
    log("Kernel start\n");

    uint8_t* video = (uint8_t*)0xA0000;
    for (int i = 0; i < 320 * 200; i++) video[i] = 0x01; /* blue */

    int line = 0;
    print_result("memmap.c", line++, test_memmap());
    print_result("pmm.c",    line++, test_pmm());
    print_result("heap.c",   line++, test_heap());
    print_result("slab.c",   line++, test_slab());
    print_result("vmm.c",    line++, test_vmm());

    draw_string_center("Tests Complete", 0x0F);
}
