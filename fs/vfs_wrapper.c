#include "../OptrixOS-Kernel/include/fs.h"

void vfs_init(void){
    fs_init();
}

fs_entry* vfs_root(void){
    return fs_get_root();
}
