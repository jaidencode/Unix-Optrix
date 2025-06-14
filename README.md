# OptrixOS

This project experiments with building a small Unix-like operating system. The
boot sector lives in `optrix_kernel/bootloader.asm` and performs the
following steps:

- Copies itself to a safe location at `0x0600` and clears low memory.
- Loads the kernel from the disk image into memory at `0x1000`.
- Initializes a simple GDT and switches the CPU to 32-bit protected mode.
- Jumps to the kernel entry point.

Use `python3 setup_bootloader.py` to assemble and link the boot files. The
script builds a small custom kernel located in `optrix_kernel/` and produces
`optrix-kernel.bin`. It requires `nasm`, `gcc` with 32-bit support, `ld`, and
`mkisofs`/`genisoimage` for ISO creation.
