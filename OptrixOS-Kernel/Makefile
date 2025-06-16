CFLAGS=-ffreestanding -fno-pie -fno-pic -m32 -Iinclude

all: disk.img

bootloader.bin: asm/bootloader.asm
	@nasm -f bin $< -o $@

kernel.bin: asm/kernel.asm src/graphics.o src/screen.o
	@nasm -f elf32 asm/kernel.asm -o kernel_asm.o
	@gcc $(CFLAGS) -c src/graphics.c -o src/graphics.o
	@gcc $(CFLAGS) -c src/screen.c -o src/screen.o
	@ld -m elf_i386 -Ttext 0x1000 kernel_asm.o src/graphics.o src/screen.o 		--oformat binary -o $@

disk.img: bootloader.bin kernel.bin
	@cat bootloader.bin kernel.bin > $@

run: disk.img
	@qemu-system-i386 -drive format=raw,file=disk.img

clean:
	@rm -f *.bin *.o disk.img src/*.o kernel_asm.o
