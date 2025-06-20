#ifndef VFS_H
#define VFS_H
#include <stddef.h>
#include "fs.h"

int vfs_open(const char* path);
size_t vfs_read(int fd, char* buf, size_t count);
int vfs_close(int fd);

#endif
