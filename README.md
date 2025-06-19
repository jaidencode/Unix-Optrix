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

The kernel runs entirely in text mode and provides a small shell interface.

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
script outputs `disk.img` and `resources.img`. The latter contains all files
from the `resources` folder. Attach both images as separate drives when running
with QEMU. For example:

```bash
qemu-system-x86_64 -drive format=raw,file=disk.img \
                   -drive format=raw,file=resources.img
```

## Built-in terminal

After boot the machine displays a plain text console. No graphics or windowing
code is present and the output matches the standard VGA text mode.
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
* `date`/`time` - show build date
* `uptime`  - show uptime counter
* `cat`     - view a file
* `touch`   - create a file
* `rm`      - delete a file
* `mv`      - rename a file
* `mkdir`   - create a directory
* `rmdir`   - remove an empty directory
* `cp`      - copy a file
* `rand`    - generate a random number
* `ver`     - show version
* `shutdown`/`exit` - halt the system

