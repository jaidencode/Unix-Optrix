#include "../OptrixOS-Kernel/include/ports.h"
void serial_write_char(char c){ outb(0x3F8, c); }
