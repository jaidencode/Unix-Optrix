CFLAGS=-ffreestanding -fno-pie -fno-pic -m32 -Iinclude

all:
	nasm -f bin asm/bootloader.asm -o bootloader.bin
	nasm -f elf32 asm/gdt.asm -o gdt.o
	nasm -f elf32 asm/idt.asm -o idt.o
	nasm -f elf32 asm/kernel_entry.asm -o kernel_entry.o
	gcc $(CFLAGS) -c kernel/main.c -o main.o
	gcc $(CFLAGS) -c kernel/terminal.c -o terminal.o
	gcc $(CFLAGS) -c kernel/graphics.c -o graphics.o
	gcc $(CFLAGS) -c kernel/keyboard.c -o keyboard.o
	gcc $(CFLAGS) -c kernel/shell.c -o shell.o
	gcc $(CFLAGS) -c kernel/debug.c -o debug.o
	gcc $(CFLAGS) -c kernel/serial.c -o serial.o
	gcc $(CFLAGS) -c kernel/pmm.c -o pmm.o
	ld -T kernel.ld -m elf_i386 kernel_entry.o gdt.o idt.o main.o terminal.o graphics.o keyboard.o shell.o debug.o serial.o pmm.o -o kernel.elf
	ld -T kernel.ld -m elf_i386 --oformat binary kernel_entry.o gdt.o idt.o main.o terminal.o graphics.o keyboard.o shell.o debug.o serial.o pmm.o -o kernel.bin
	cat bootloader.bin kernel.bin > disk.img

run: all
	qemu-system-i386 -drive format=raw,file=disk.img
