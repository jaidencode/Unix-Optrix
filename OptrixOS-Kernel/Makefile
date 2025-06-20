CFLAGS=-ffreestanding -fno-pie -fno-pic -m32 -Iinclude
	
all: disk.img
	
bootloader.bin: asm/bootloader.asm
	@nasm -f bin $< -o $@
	
kernel.bin: asm/kernel.asm asm/ports.asm
	@nasm -f elf32 asm/kernel.asm -o kernel_asm.o
	@nasm -f elf32 asm/ports.asm -o ports.o
	@gcc $(CFLAGS) -c src/screen.c -o src/screen.o
	@gcc $(CFLAGS) -c src/keyboard.c -o src/keyboard.o
	@gcc $(CFLAGS) -c src/terminal.c -o src/terminal.o
	@gcc $(CFLAGS) -c src/fs.c -o src/fs.o
	@gcc $(CFLAGS) -c src/disk.c -o src/disk.o
	@gcc $(CFLAGS) -c src/root_files.c -o src/root_files.o
	@gcc $(CFLAGS) -c src/driver.c -o src/driver.o
	@gcc $(CFLAGS) -c src/mem.c -o src/mem.o
	@gcc $(CFLAGS) -c src/kernel_main.c -o src/kernel_main.o
	@gcc $(CFLAGS) -c ../drivers/keyboard_drv.c -o drivers_keyboard_drv.o
	@gcc $(CFLAGS) -c ../fs/vfs_wrapper.c -o fs_vfs_wrapper.o
	@gcc $(CFLAGS) -c ../hardware/timer.c -o hardware_timer.o
	@gcc $(CFLAGS) -c ../io/serial.c -o io_serial.o
	@gcc $(CFLAGS) -c ../libc/string.c -o libc_string.o
	@gcc $(CFLAGS) -c ../misc/util.c -o misc_util.o
	@gcc $(CFLAGS) -c ../shell/shell_entry.c -o shell_shell_entry.o
	@gcc $(CFLAGS) -c ../sys/system.c -o sys_system.o
	@gcc $(CFLAGS) -c ../system/init.c -o system_init.o
	@gcc $(CFLAGS) -c ../kernel.c -o kernel.o
	@ld -m elf_i386 -Ttext 0x1000 kernel_asm.o ports.o \
src/screen.o src/keyboard.o src/terminal.o src/fs.o src/disk.o \
src/root_files.o src/driver.o src/mem.o src/kernel_main.o \
drivers_keyboard_drv.o fs_vfs_wrapper.o hardware_timer.o \
io_serial.o libc_string.o misc_util.o shell_shell_entry.o \
sys_system.o system_init.o kernel.o --oformat binary -o $@
	
disk.img: bootloader.bin kernel.bin
	@cat bootloader.bin kernel.bin > $@
	
run: disk.img
	@qemu-system-i386 -drive format=raw,file=disk.img
	
clean:
	@rm -f *.bin *.o disk.img src/*.o kernel_asm.o kernel.o
