#ifndef VFS_H
#define VFS_H
#include "fs.h"
#include <stddef.h>

#define VFS_MAX_OPEN 16

typedef struct {
    fs_entry* entry;
    size_t offset;
    int flags;
} vfs_handle;

int vfs_open(const char* path, int flags);
int vfs_close(int fd);
size_t vfs_read(int fd, char* buf, size_t n);
size_t vfs_write(int fd, const char* buf, size_t n);

#endif
