void kmain(void) {
    const char *msg = "Optrix kernel loaded";
    char *video = (char*)0xb8000;
    for (int i = 0; msg[i]; i++) {
        video[i*2] = msg[i];
        video[i*2 + 1] = 0x07;
    }
    for(;;);
}
