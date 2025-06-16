# Minimal Graphics OS

This repository contains a simple bootloader and kernel that boot into
1920x1080 graphics mode and draw a white border around a black screen.

## Building

Make sure `nasm`, `gcc` and `ld` are installed. Build the image with:

```bash
make
```

This produces `os-image.bin` which can be run with QEMU:

```bash
qemu-system-i386 -fda os-image.bin
```

The `Run_OptrixOS.bat` script launches QEMU with the generated image on
Windows.
