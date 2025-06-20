#ifndef DRIVER_H
#define DRIVER_H

typedef struct {
    const char *name;
    void (*init)(void);
    void (*update)(void);
} driver_t;

void driver_register(driver_t *drv);
void driver_init_all(void);
void driver_update_all(void);
void driver_setup(void);

#endif
