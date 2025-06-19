CFLAGS=-ffreestanding -fno-pie -fno-pic -m32 -Iinclude
	
all: boot.img resources.img
	
bootloader.bin: asm/bootloader.asm
	@nasm -f bin $< -o $@
	
kernel.bin: asm/kernel.asm asm/ports.asm src/screen.o src/keyboard.o src/terminal.o src/fs.o src/driver.o src/mem.o src/kernel_main.o src/disk.o src/resources_test.o src/embedded_resources.o
	@nasm -f elf32 asm/kernel.asm -o kernel_asm.o
	@nasm -f elf32 asm/ports.asm -o ports.o
	@gcc $(CFLAGS) -c src/screen.c -o src/screen.o
	@gcc $(CFLAGS) -c src/keyboard.c -o src/keyboard.o
	@gcc $(CFLAGS) -c src/terminal.c -o src/terminal.o
	@gcc $(CFLAGS) -c src/fs.c -o src/fs.o
	@gcc $(CFLAGS) -c src/driver.c -o src/driver.o
	@gcc $(CFLAGS) -c src/mem.c -o src/mem.o
	@gcc $(CFLAGS) -c src/kernel_main.c -o src/kernel_main.o
	@gcc $(CFLAGS) -c src/disk.c -o src/disk.o
        @gcc $(CFLAGS) -c src/resources_test.c -o src/resources_test.o
        @gcc $(CFLAGS) -c src/embedded_resources.c -o src/embedded_resources.o
	@ld -m elf_i386 -Ttext 0x1000 kernel_asm.o ports.o \
src/screen.o src/keyboard.o src/terminal.o src/fs.o \
src/driver.o src/mem.o src/kernel_main.o src/disk.o src/resources_test.o src/embedded_resources.o --oformat binary -o $@
	
boot.img: bootloader.bin kernel.bin
        @cat bootloader.bin kernel.bin > $@

resources.img:
        @echo "Use setup_bootloader.py to create resources.img"
	
run: boot.img resources.img
        @qemu-system-i386 -drive format=raw,file=boot.img -drive format=raw,file=resources.img,media=disk,if=ide,index=1
	
clean:
        @rm -f *.bin *.o boot.img resources.img src/*.o kernel_asm.o
