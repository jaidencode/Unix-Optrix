all: os-image.bin

os-image.bin: bootloader.bin kernel.bin
	cat bootloader.bin kernel.bin > os-image.bin

bootloader.bin: boot/bootloader.asm
	nasm -f bin boot/bootloader.asm -o bootloader.bin

kernel.bin: src/kernel.o src/handler.o link.ld
	ld -T link.ld -melf_i386 src/kernel.o src/handler.o -o kernel.bin --oformat binary

src/kernel.o: src/kernel.c include/handler.h
	gcc -m32 -ffreestanding -fno-pie -no-pie -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -nostdlib -Iinclude -c src/kernel.c -o src/kernel.o

src/handler.o: src/handler.c include/handler.h
	gcc -m32 -ffreestanding -fno-pie -no-pie -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -nostdlib -Iinclude -c src/handler.c -o src/handler.o

clean:
	rm -f *.bin src/*.o os-image.bin
