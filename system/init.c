#include "../OptrixOS-Kernel/include/screen.h"
#include "../OptrixOS-Kernel/include/mem.h"
#include "../OptrixOS-Kernel/include/disk.h"
#include "../OptrixOS-Kernel/include/fs.h"
#include "../OptrixOS-Kernel/include/driver.h"
#include "../OptrixOS-Kernel/include/terminal.h"

void system_init(void){
    screen_init();
    mem_init((unsigned char*)0x200000, 64*1024);
    disk_init();
    fs_init();
    driver_setup();
    driver_init_all();
    terminal_init();
}
