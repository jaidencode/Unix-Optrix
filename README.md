# OptrixOS

This project experiments with building a small Unix-like operating system. The
initial bootloader is written in NASM and performs the following steps:

- Loads the kernel from the disk image into memory at `0x1000`.
- Initializes a simple GDT and switches the CPU to 32-bit protected mode.
- Jumps to the kernel entry point.

Use `python3 setup_bootloader.py` to assemble and link the boot files. The
script builds a small custom kernel located in `optrix_kernel/` and produces
`optrix-kernel.bin`. It requires `nasm`, `gcc` with 32-bit support, and `ld`.
