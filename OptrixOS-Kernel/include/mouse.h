#ifndef MOUSE_H
#define MOUSE_H

void mouse_init(void);
void mouse_update(void);
int mouse_get_x(void);
int mouse_get_y(void);
int mouse_clicked(void);
int mouse_is_present(void);

#endif
