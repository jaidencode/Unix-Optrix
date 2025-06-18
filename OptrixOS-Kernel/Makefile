CFLAGS=-ffreestanding -fno-pie -fno-pic -m32 -Iinclude

# This Makefile simply forwards to the Python build script so that
# users building via `make` also get the initrd and resources embedded.

.PHONY: all iso run clean

all: iso

# Build the bootable image and ISO using the helper script
iso:
	python3 ../setup_bootloader.py

# Run the resulting ISO with QEMU
run: iso
	qemu-system-i386 -cdrom ../OptrixOS.iso

clean:
	rm -rf _build_obj bootloader.bin OptrixOS-kernel.bin \
	OptrixOS-kernel.elf disk.img ../OptrixOS.iso
