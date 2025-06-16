# OptrixOS

This project experiments with building a small Unix-like operating system. The
boot sector is defined in `OptrixOS-Kernel/bootloader.asm` and now mimics the
behaviour of the historic `fsboot` loader. When executed it:

- Copies itself away from `0x7c00` so memory can be cleared.
- Clears a portion of RAM before loading the kernel.
- Prompts for a kernel path (the input is currently ignored but demonstrates
  the interface).
- Loads the kernel from the disk image into memory at `0x1000`.
- Initializes a simple GDT and switches the CPU to 32-bit protected mode.
- Jumps to the kernel entry point.
- Uses the VESA linear framebuffer so the full 1920x1080 screen is accessible.
- Bootloader prints progress messages while loading the kernel.
- Displays a simple spinning logo for a few seconds before launching the
  terminal.

Once the kernel takes over it switches to a graphical mode and
initialises a very small shell interface.

Use `python3 setup_bootloader.py` to assemble and link the boot files. The
script builds a small custom kernel located in `OptrixOS-Kernel/` and produces
`OptrixOS-kernel.bin`. A cross compiler (`i686-linux-gnu-gcc`/`ld`) is preferred,
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

## Built-in terminal

After boot a simple text terminal is available. The screen now runs in a
high‑resolution 1920x1080 graphics mode with characters rendered at an
8‑pixel size. A title bar with a box-drawing border is drawn using VGA graphics
characters. The background is white with black text while the cursor is
rendered in bright yellow. The hardware text mode cursor is disabled so only the
custom cursor is visible. The terminal automatically scrolls as it fills.
The following commands are implemented:

* `help`    - display available commands
* `clear`   - clear the screen
* `echo`    - echo arbitrary text
* `about`   - display information about OptrixOS
* `ping`    - check connectivity
* `reverse` - reverse a string
* `add`     - add two numbers
* `color`   - set the text colour
* `border`  - redraw the terminal border
* `dir`     - list directory contents
* `cd`      - change directory
* `pwd`     - show current directory
* `date`    - show build date
* `whoami`  - display current user
* `hello`   - greet the user
* `uptime`  - show uptime counter
* `cat`     - view a file
* `touch`   - create a file
* `write`   - write text to a file
* `rm`      - delete a file
* `mv`      - rename a file
* `mkdir`   - create a directory
* `rmdir`   - remove an empty directory
  (directory capacity has been increased to support more entries)

