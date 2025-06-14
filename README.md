# OptrixOS

This project experiments with building a small Unix-like operating system. The
boot sector is defined in `optrix_kernel/bootloader.asm` and now mimics the
behaviour of the historic `fsboot` loader. When executed it:

- Copies itself away from `0x7c00` so memory can be cleared.
- Clears a portion of RAM before loading the kernel.
- Prompts for a kernel path (the input is currently ignored but demonstrates
  the interface).
- Loads the kernel from the disk image into memory at `0x1000`.
- Initializes a simple GDT and switches the CPU to 32-bit protected mode.
- Jumps to the kernel entry point.

Use `python3 setup_bootloader.py` to assemble and link the boot files. The
script builds a small custom kernel located in `optrix_kernel/` and produces
`optrix-kernel.bin`. A cross compiler (`i686-linux-gnu-gcc`/`ld`) is preferred,
but if it is not installed the script will fall back to the system `gcc` and
`ld` with `-m32`.

On Ubuntu these tools can be installed with:

```bash
sudo apt-get install gcc-i686-linux-gnu binutils-i686-linux-gnu genisoimage
```

Build the bootable image with:

```bash
python3 setup_bootloader.py
```

If `mkisofs` is available an ISO named `OptrixOS.iso` is created. Otherwise the
script outputs `disk.img` which can be run with:

```bash
qemu-system-x86_64 -hda disk.img
```
