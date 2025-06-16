CFLAGS=-ffreestanding -fno-pie -fno-pic -m32 -Iinclude

all: disk.img

bootloader.bin: asm/bootloader.asm
	@nasm -f bin $< -o $@

kernel.bin: asm/kernel.asm asm/ports.asm src/graphics.o src/screen.o src/keyboard.o src/terminal.o src/fs.o src/boot_logo.o
	@nasm -f elf32 asm/kernel.asm -o kernel_asm.o
	@nasm -f elf32 asm/ports.asm -o ports.o
	@gcc $(CFLAGS) -c src/graphics.c -o src/graphics.o
	@gcc $(CFLAGS) -c src/screen.c -o src/screen.o
	@gcc $(CFLAGS) -c src/keyboard.c -o src/keyboard.o
	@gcc $(CFLAGS) -c src/terminal.c -o src/terminal.o
	@gcc $(CFLAGS) -c src/fs.c -o src/fs.o
	@gcc $(CFLAGS) -c src/boot_logo.c -o src/boot_logo.o
	@ld -m elf_i386 -Ttext 0x1000 kernel_asm.o ports.o src/graphics.o src/screen.o src/keyboard.o src/terminal.o src/fs.o src/boot_logo.o --oformat binary -o $@

disk.img: bootloader.bin kernel.bin
	@cat bootloader.bin kernel.bin > $@

run: disk.img
	@qemu-system-i386 -drive format=raw,file=disk.img

clean:
	@rm -f *.bin *.o disk.img src/*.o kernel_asm.o
