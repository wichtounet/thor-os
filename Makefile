default: thor.flp

KERNEL_SRC=$(wildcard micro_kernel/*.asm)
KERNEL_UTILS_SRC=$(wildcard micro_kernel/utils/*.asm)

bootloader.bin: bootloader/bootloader.asm
	nasm -w+all -f bin -o bootloader.bin bootloader/bootloader.asm

micro_kernel.bin: $(KERNEL_SRC) $(KERNEL_UTILS_SRC)
	nasm -w+all -f bin -o micro_kernel.bin micro_kernel/micro_kernel.asm
	nasm -D DEBUG -g -w+all -f elf64 -o micro_kernel.g micro_kernel/micro_kernel.asm

kernel.o: kernel/src/kernel.cpp
	g++ -masm=intel -Ikernel/include/ -O2 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti -ffreestanding -c kernel/src/kernel.cpp -o kernel.o

kernel.bin: kernel.o
	g++ -std=c++11 -T linker.ld -o kernel.bin.o -ffreestanding -O2 -nostdlib kernel.o
	objcopy -R .note -R .comment -S -O binary kernel.bin.o kernel.bin

filler.bin: kernel.bin
	bash fill.bash

thor.flp: bootloader.bin micro_kernel.bin kernel.bin filler.bin
	cat bootloader.bin > thor.bin
	cat micro_kernel.bin >> thor.bin
	cat kernel.bin >> thor.bin
	cat filler.bin >> thor.bin
	dd status=noxfer conv=notrunc if=thor.bin of=thor.flp

qemu: thor.flp
	qemu-kvm -cpu host -fda thor.flp

bochs: thor.flp
	bochs -q -f bochsrc.txt

clean:
	rm -f *.bin
	rm -f *.flp
	rm -f *.o
	rm -f *.g