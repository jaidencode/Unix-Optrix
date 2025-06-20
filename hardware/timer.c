#include <stdint.h>
volatile unsigned int timer_ticks = 0;
void timer_tick(void){ timer_ticks++; }
unsigned int timer_get_ticks(void){ return timer_ticks; }
