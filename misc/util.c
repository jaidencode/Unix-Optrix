#include "../OptrixOS-Kernel/include/ports.h"
#include "../io/serial.h"
#include <stdint.h>

void util_beep(void){
    serial_write_char('\a');
    uint8_t tmp = inb(0x61);
    outb(0x61, tmp | 3);
    for(volatile int i=0;i<10000;i++);
    outb(0x61, tmp & 0xFC);
}
void util_beep(void){ /* placeholder */ }
