#ifndef WALLPAPER_H
#define WALLPAPER_H
#include <stdint.h>
#include <stddef.h>

uint32_t* load_wallpaper_from_iso(const char* path, int* width, int* height);
void free_wallpaper(uint32_t* buf);

#endif // WALLPAPER_H
