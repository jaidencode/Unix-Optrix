#include "system/init.h"
#include "shell/shell_entry.h"

/* Simple wrapper around existing kernel_main logic */
void kernel_main(void);

void kmain(void){
    system_init();
    shell_run();
}
