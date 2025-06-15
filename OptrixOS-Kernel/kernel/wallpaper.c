#include "wallpaper.h"
#include "iso9660.h"
#include "pmm.h"

#include <stddef.h>
#include <stdint.h>

void* kernel_malloc(size_t size) { return pmm_alloc(); }
void kernel_free(void* ptr) { (void)ptr; }
void* kernel_realloc(void* ptr, size_t size) { (void)ptr; (void)size; return NULL; }

#define STBI_MALLOC(sz)      kernel_malloc(sz)
#define STBI_REALLOC(p,sz)   kernel_realloc(p,sz)
#define STBI_FREE(p)         kernel_free(p)
#define STBI_NO_STDIO
#define STBI_NO_LIMITS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

uint32_t* load_wallpaper_from_iso(const char* path, int* width, int* height) {
    size_t filesize = 0;
    uint8_t* file_data = (uint8_t*)iso9660_load_file(path, &filesize);
    if (!file_data || filesize == 0) {
        if (width) *width = 0;
        if (height) *height = 0;
        return NULL;
    }
    int w = 0, h = 0, comp = 0;
    unsigned char* pixels = stbi_load_from_memory(file_data, filesize, &w, &h, &comp, 4);
    if (!pixels) {
        if (width) *width = 0;
        if (height) *height = 0;
        return NULL;
    }
    if (width) *width = w;
    if (height) *height = h;
    return (uint32_t*)pixels;
}

void free_wallpaper(uint32_t* buf) {
    if (buf) stbi_image_free(buf);
}
