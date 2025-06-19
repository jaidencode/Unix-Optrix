# OptrixOS

This project experiments with building a small Unix-like operating system. The
boot sector is defined in `OptrixOS-Kernel/bootloader.asm` and now mimics the
behaviour of the historic `fsboot` loader. When executed it:

- Copies itself away from `0x7c00` so memory can be cleared.
- Clears a portion of RAM before loading the kernel.
- Prompts for a kernel path (the input is currently ignored but demonstrates
  the interface).
- Loads the kernel from disk into memory at `0x1000`.
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

Running `python3 setup_bootloader.py` now produces `OptrixOS.iso` directly.
The script creates a `boot.img` containing the bootloader and kernel, which is
used as the El Torito boot image so the loader can read the kernel from
contiguous sectors.
The kernel no longer includes a filesystem. It boots directly into a minimal terminal with built-in keyboard and graphics drivers.

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
* `date`/`time` - show build date


* `rand`    - generate a random number
* `uptime` - show uptime counter
* `ver`     - show version
* `shutdown` - halt the system

A total of 50 commands are built in; the remaining ones are simple placeholders for future features.
