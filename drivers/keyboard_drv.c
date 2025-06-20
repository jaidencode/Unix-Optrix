#include "../OptrixOS-Kernel/include/keyboard.h"
#include "../OptrixOS-Kernel/include/driver.h"

static void kb_init(void) {}
static void kb_update(void) { keyboard_update(); }

driver_t keyboard_driver = { "keyboard", kb_init, kb_update };
