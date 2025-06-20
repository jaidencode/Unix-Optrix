#include "vfs.h"
#include "mem.h"
#include <stddef.h>

static vfs_handle handles[VFS_MAX_OPEN];

static size_t str_len(const char*s){size_t l=0;while(s[l])l++;return l;}
static int is_sep(char c){return c=='/'||c=='\\';}

static fs_entry* resolve(fs_entry* base,const char* path,int create){
    char name[32];int idx=0;
    while(*path){
        while(*path && is_sep(*path)) path++;
        if(!*path) break;
        idx=0;
        while(*path && !is_sep(*path) && idx<31){name[idx++]=*path++;}
        name[idx]=0;
        if(*path){
            fs_entry* d=fs_find_subdir(base,name);
            if(!d) return NULL;
            base=d;
        }else{
            fs_entry* f=fs_find_entry(base,name);
            if(!f && create) f=fs_create_file(base,name);
            return f;
        }
    }
    return base;
}

int vfs_open(const char* path, int flags){
    for(int i=0;i<VFS_MAX_OPEN;i++) if(!handles[i].entry){
        fs_entry* e = resolve(fs_get_root(), path, flags&1/*O_CREAT*/);
        if(!e) return -1;
        handles[i].entry=e; handles[i].offset=0; handles[i].flags=flags; return i;
    }
    return -1;
}

int vfs_close(int fd){
    if(fd<0||fd>=VFS_MAX_OPEN||!handles[fd].entry) return -1;
    handles[fd].entry=NULL; handles[fd].offset=0; handles[fd].flags=0; return 0;
}

size_t vfs_read(int fd, char* buf, size_t n){
    if(fd<0||fd>=VFS_MAX_OPEN) return 0;
    vfs_handle*h=&handles[fd];
    if(!h->entry || h->entry->is_dir) return 0;
    const char* data=fs_read_file(h->entry);
    size_t len=str_len(data);
    if(h->offset>=len) return 0;
    if(h->offset+n>len) n=len-h->offset;
    for(size_t i=0;i<n;i++) buf[i]=data[h->offset+i];
    h->offset+=n; return n;
}

static void write_replace(fs_entry* f,const char* data){
    fs_write_file(f,data);
}

size_t vfs_write(int fd, const char* buf, size_t n){
    if(fd<0||fd>=VFS_MAX_OPEN) return 0;
    vfs_handle*h=&handles[fd];
    if(!h->entry || h->entry->is_dir) return 0;
    char old[256]; const char* c=fs_read_file(h->entry); size_t len=str_len(c);
    if(len>255) len=255; for(size_t i=0;i<len;i++) old[i]=c[i]; old[len]=0;
    if(h->offset>len) h->offset=len;
    if(len+n>255) n=255-len;
    for(size_t i=len;i>h->offset;i--) old[i+n-1]=old[i-1];
    for(size_t i=0;i<n;i++) old[h->offset+i]=buf[i];
    old[len+n]=0;
    write_replace(h->entry,old);
    h->offset+=n; return n;
}
