#ifndef DESKTOP_H
#define DESKTOP_H

void desktop_init(void);
void desktop_run(void);
/* Redraws a portion of the desktop wallpaper and icons. */
void desktop_redraw_region(int x, int y, int w, int h);

#endif
