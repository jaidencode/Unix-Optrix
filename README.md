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
- Sets the classic 80x25 text mode and jumps directly to the kernel.

The kernel now enables a basic 320x200 graphics mode before switching to the
text console. A minimal in-memory filesystem is created at boot using dynamic
structures rather than the previous static C arrays.

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
python3 setup_bootloader.py --iso my_image.iso
```

The build script tries to locate `mkisofs` or the `genisoimage` alternative. If
either is available an ISO named `OptrixOS.iso` is produced. When none are
found the step is skipped and only `disk.img` is generated which can be run with:

```bash
qemu-system-x86_64 -hda disk.img
```

## Built-in terminal

After boot the machine switches to a basic VGA graphics mode and then drops to
the text console. No windowing system exists yet but graphics primitives are
available for future use.
The following commands are implemented:

* `help`    - display available commands
* `clear`/`cls` - clear the screen
* `echo`    - echo arbitrary text
* `about`   - display information about OptrixOS
* `add`     - add two numbers
* `mul`     - multiply two numbers
* `dir`/`ls`- list directory contents
* `cd`      - change directory
* `pwd`     - show current directory
  Paths may include `/`, `.` and `..` components.
* `date`/`time` - show build date
* `uptime`  - show uptime counter
* `cat`     - view a file
* `touch`   - create a file
* `rm`      - delete a file
* `mv`      - rename a file
* `mkdir`   - create a directory
* `rmdir`   - remove an empty directory
* `cp`      - copy a file
  Most file commands understand paths with `/`, `.` and `..`.
* `rand`    - generate a random number
* `ver`     - show version
* `shutdown`/`exit` - halt the system

