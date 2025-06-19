CFLAGS=-ffreestanding -fno-pie -fno-pic -m32 -Iinclude

all: kernel.bin

kernel.bin: asm/bootloader.asm asm/kernel.asm asm/ports.asm \
            src/screen.o src/keyboard.o src/graphics.o \
            src/terminal.o src/driver.o src/mem.o src/kernel_main.o
	nasm -felf32 asm/kernel.asm -o kernel_asm.o
	nasm -felf32 asm/ports.asm -o ports.o
	gcc $(CFLAGS) -c src/screen.c -o src/screen.o
	gcc $(CFLAGS) -c src/keyboard.c -o src/keyboard.o
	gcc $(CFLAGS) -c src/graphics.c -o src/graphics.o
	gcc $(CFLAGS) -c src/terminal.c -o src/terminal.o
	gcc $(CFLAGS) -c src/driver.c -o src/driver.o
	gcc $(CFLAGS) -c src/mem.c -o src/mem.o
	gcc $(CFLAGS) -c src/kernel_main.c -o src/kernel_main.o
	ld -m elf_i386 -Ttext 0x1000 kernel_asm.o ports.o src/screen.o src/keyboard.o src/graphics.o src/terminal.o src/driver.o src/mem.o src/kernel_main.o --oformat binary -o $@

clean:
rm -f *.bin *.o src/*.o kernel_asm.o ports.o
