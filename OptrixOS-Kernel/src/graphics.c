#include "graphics.h"

void graphics_mode_init(void){
    __asm__ __volatile__(
        "mov $0x13, %%ax\n"
        "int $0x10\n"
        : : : "ax"
    );
}
