#include "vfs.h"
#include <stddef.h>

#define MAX_OPEN_FILES 16

typedef struct {
    fs_entry* file;
    size_t offset;
} fd_entry;

static fd_entry fds[MAX_OPEN_FILES];

int vfs_open(const char* path){
    fs_entry* dir = fs_get_root();
    char part[32];
    int pi = 0;
    for(size_t i=0;;i++){
        char c = path[i];
        if(c=='/' || c==0){
            part[pi]=0;
            if(part[0]){
                fs_entry* next = fs_find_entry(dir, part);
                if(!next)
                    return -1;
                dir = next;
            }
            pi=0;
            if(c==0) break;
        } else if(pi<31){
            part[pi++] = c;
        }
    }
    if(!dir || dir->is_dir)
        return -1;
    for(int i=0;i<MAX_OPEN_FILES;i++){
        if(fds[i].file==NULL){
            fds[i].file = dir;
            fds[i].offset = 0;
            return i;
        }
    }
    return -1;
}

size_t vfs_read(int fd, char* buf, size_t count){
    if(fd<0 || fd>=MAX_OPEN_FILES) return 0;
    fd_entry* e = &fds[fd];
    if(!e->file || e->file->is_dir || !e->file->content) return 0;
    size_t remaining = e->file->size > e->offset ? e->file->size - e->offset : 0;
    if(count > remaining) count = remaining;
    for(size_t i=0;i<count;i++)
        buf[i] = e->file->content[e->offset + i];
    e->offset += count;
    return count;
}

int vfs_close(int fd){
    if(fd<0 || fd>=MAX_OPEN_FILES) return -1;
    fds[fd].file = NULL;
    fds[fd].offset = 0;
    return 0;
}
