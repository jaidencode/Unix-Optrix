#include "fs.h"
#include "bootinfo.h"
#include <stdint.h>
#include <stddef.h>

static int streq(const char* a, const char* b) {
    while(*a && *b) {
        if(*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

#define MAX_ROOT_ENTRIES 20
static fs_entry root_entries[MAX_ROOT_ENTRIES];
static int root_count = 0;

#define MAX_BIN_ENTRIES 5
static fs_entry bin_entries[MAX_BIN_ENTRIES];
static int bin_count = 0;

#define MAX_DOC_ENTRIES 5
static fs_entry docs_entries[MAX_DOC_ENTRIES];
static int docs_count = 0;

/* entries for /desktop */
#define MAX_DESKTOP_ENTRIES 20
static fs_entry desktop_entries[MAX_DESKTOP_ENTRIES];
static int desktop_count = 0;

static fs_entry root_dir = {"/", 1, NULL, root_entries, 0, ""};

#define MAX_EXTRA_DIRS 20
#define MAX_EXTRA_DIR_ENTRIES 20
static fs_entry extra_dir_entries[MAX_EXTRA_DIRS][MAX_EXTRA_DIR_ENTRIES];
static int extra_dir_used = 0;

fs_entry* fs_get_root(void) {
    return &root_dir;
}

void fs_init(const void* initrd_start, unsigned int initrd_size) {
    root_count = 0;
    bin_count = 0;
    docs_count = 0;
    desktop_count = 0;

    /* setup /bin directory */
    fs_entry bin = {"bin", 1, &root_dir, bin_entries, 0, ""};
    root_entries[root_count++] = bin;

    /* setup /docs directory */
    fs_entry docs = {"docs", 1, &root_dir, docs_entries, 0, ""};
    root_entries[root_count++] = docs;

    /* setup /desktop directory */
    fs_entry desktop = {"desktop", 1, &root_dir, desktop_entries, 0, ""};
    root_entries[root_count++] = desktop;
    fs_entry *desktop_ptr = &root_entries[root_count-1];

    /* simple readme */
    fs_entry readme = {"readme.txt", 0, &root_dir, NULL, 0, "Welcome to OptrixOS"};
    root_entries[root_count++] = readme;

    /* /desktop/terminal.opt executable */
    fs_entry term = {"terminal.opt", 0, desktop_ptr, NULL, 0, ""};
    desktop_entries[desktop_count++] = term;
    desktop_ptr->child_count = desktop_count;

    root_dir.child_count = root_count;

    /* load files from initrd */
    if(initrd_start && initrd_size >= 4) {
        const uint8_t* ptr = (const uint8_t*)initrd_start;
        uint32_t count = *(const uint32_t*)ptr; ptr += 4;
        for(uint32_t i=0; i<count && root_dir.child_count < MAX_ROOT_ENTRIES; i++) {
            if(ptr + 36 > (const uint8_t*)initrd_start + initrd_size) break;
            char namebuf[33];
            for(int j=0;j<32;j++) namebuf[j]=ptr[j];
            namebuf[32]='\0';
            ptr += 32;
            uint32_t sz = *(const uint32_t*)ptr; ptr += 4;
            if(ptr + sz > (const uint8_t*)initrd_start + initrd_size) break;
            fs_entry* f = fs_create_file(&root_dir, namebuf);
            if(f) {
                char text[256];
                uint32_t cpy = sz < 255 ? sz : 255;
                for(uint32_t k=0;k<cpy;k++) text[k] = ((const char*)ptr)[k];
                text[cpy]='\0';
                fs_write_file(f, text);
            }
            ptr += sz;
        }
    }
}

fs_entry* fs_find_subdir(fs_entry* dir, const char* name) {
    for(int i=0; i<dir->child_count; i++)
        if(dir->children[i].is_dir && name && dir->children[i].name &&
           streq(dir->children[i].name, name))
            return (fs_entry*)&dir->children[i];
    return 0;
}

fs_entry* fs_find_entry(fs_entry* dir, const char* name) {
    for(int i=0; i<dir->child_count; i++)
        if(name && streq(dir->children[i].name, name))
            return &dir->children[i];
    return 0;
}

fs_entry* fs_create_file(fs_entry* dir, const char* name) {
    int limit = (dir == &root_dir) ? MAX_ROOT_ENTRIES : MAX_EXTRA_DIR_ENTRIES;
    if(dir->child_count >= limit) return 0;
    fs_entry* e = &dir->children[dir->child_count++];
    for(int i=0;i<32;i++){e->name[i]=0;}
    int idx=0; while(name[idx] && idx<31){ e->name[idx]=name[idx]; idx++; }
    e->name[idx]='\0';
    e->is_dir = 0;
    e->parent = dir;
    e->children = NULL;
    e->child_count = 0;
    e->content[0]='\0';
    return e;
}

fs_entry* fs_create_dir(fs_entry* dir, const char* name) {
    int limit = (dir == &root_dir) ? MAX_ROOT_ENTRIES : MAX_EXTRA_DIR_ENTRIES;
    if(dir->child_count >= limit) return 0;
    if(extra_dir_used >= MAX_EXTRA_DIRS) return 0;
    fs_entry* e = &dir->children[dir->child_count++];
    for(int i=0;i<32;i++){e->name[i]=0;}
    int idx=0; while(name[idx] && idx<31){ e->name[idx]=name[idx]; idx++; }
    e->name[idx]='\0';
    e->is_dir = 1;
    e->parent = dir;
    e->children = extra_dir_entries[extra_dir_used];
    e->child_count = 0;
    e->content[0]='\0';
    extra_dir_used++;
    return e;
}

int fs_delete_entry(fs_entry* dir, const char* name) {
    for(int i=0;i<dir->child_count;i++) {
        if(streq(dir->children[i].name,name)) {
            for(int j=i;j<dir->child_count-1;j++)
                dir->children[j]=dir->children[j+1];
            dir->child_count--;
            return 1;
        }
    }
    return 0;
}

void fs_write_file(fs_entry* file, const char* text) {
    if(!file || file->is_dir) return;
    int k = 0;
    while(text[k] && k < 255) {
        file->content[k] = text[k];
        k++;
    }
    file->content[k] = '\0';
}

const char* fs_read_file(fs_entry* file) {
    if(!file || file->is_dir) return "";
    return file->content;
}

fs_entry* fs_resolve(fs_entry* start, const char* path) {
    if(!start) start = &root_dir;
    if(!path || !*path) return start;
    fs_entry* cur = start;
    int i = 0;
    char seg[32];
    int seglen = 0;
    if(path[0] == '/') {
        cur = &root_dir;
        i = 1;
    }
    while(1) {
        char c = path[i];
        if(c == '/' || c == '\0') {
            seg[seglen] = '\0';
            if(seglen > 0) {
                if(streq(seg, ".")) {
                    /* noop */
                } else if(streq(seg, "..")) {
                    if(cur->parent) cur = cur->parent;
                } else {
                    fs_entry* next = fs_find_entry(cur, seg);
                    if(!next) return NULL;
                    cur = next;
                }
            }
            seglen = 0;
            if(c == '\0') break;
            i++;
        } else {
            if(seglen < 31) {
                seg[seglen++] = c;
            }
            i++;
        }
    }
    return cur;
}

void fs_get_path(fs_entry* entry, char* out, int max_len) {
    if(max_len <= 0) return;
    if(entry == &root_dir) {
        if(max_len >= 2) {
            out[0] = '/';
            out[1] = '\0';
        } else if(max_len >= 1) {
            out[0] = '\0';
        }
        return;
    }
    fs_entry* stack[16];
    int depth = 0;
    fs_entry* cur = entry;
    while(cur && cur != &root_dir && depth < 16) {
        stack[depth++] = cur;
        cur = cur->parent;
    }
    int idx = 0;
    out[idx++] = '/';
    for(int d = depth - 1; d >= 0 && idx < max_len - 1; d--) {
        const char* n = stack[d]->name;
        int j = 0;
        while(n[j] && idx < max_len - 1) {
            out[idx++] = n[j++];
        }
        if(d > 0 && idx < max_len - 1) {
            out[idx++] = '/';
        }
    }
    out[idx] = '\0';
}
