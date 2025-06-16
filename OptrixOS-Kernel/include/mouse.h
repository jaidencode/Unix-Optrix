#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

void mouse_init(void);
void mouse_update(void);
int mouse_get_x(void);
int mouse_get_y(void);
int mouse_clicked(void);
int mouse_is_present(void);
void mouse_set_visible(int vis);
int mouse_get_visible(void);
void mouse_draw(uint8_t bg_color);

#endif
