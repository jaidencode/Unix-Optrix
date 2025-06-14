all:
	nasm -f bin asm/bootloader.asm -o bootloader.bin
	nasm -f elf32 asm/gdt.asm -o gdt.o
	nasm -f elf32 asm/idt.asm -o idt.o
	nasm -f elf32 asm/kernel_entry.asm -o kernel_entry.o
	gcc -ffreestanding -m32 -c kernel/main.c -o main.o
	gcc -ffreestanding -m32 -c kernel/vga.c -o vga.o
	gcc -ffreestanding -m32 -c kernel/serial.c -o serial.o
	gcc -ffreestanding -m32 -c kernel/pmm.c -o pmm.o
	ld -T kernel.ld -m elf_i386 kernel_entry.o gdt.o idt.o main.o vga.o serial.o pmm.o -o kernel.elf
	ld -T kernel.ld -m elf_i386 --oformat binary kernel_entry.o gdt.o idt.o main.o vga.o serial.o pmm.o -o kernel.bin
	cat bootloader.bin kernel.bin > disk.img

run: all
	qemu-system-i386 -drive format=raw,file=disk.img
