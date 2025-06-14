void kernel_main(void) {
    char *video = (char*)0xb8000;
    video[0] = 'O';
    video[1] = 0x07;
    for(;;);
}
