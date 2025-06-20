#ifndef VFS_WRAPPER_H
#define VFS_WRAPPER_H
#include "../OptrixOS-Kernel/include/fs.h"
void vfs_init(void);
fs_entry* vfs_root(void);
#endif
