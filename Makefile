default: thor.flp

KERNEL_SRC=$(wildcard src/*.asm)
KERNEL_UTILS_SRC=$(wildcard src/utils/*.asm)

bootloader.bin: src/bootloader/bootloader.asm
	nasm -w+all -f bin -o bootloader.bin src/bootloader/bootloader.asm

micro_kernel.bin: $(KERNEL_SRC) $(KERNEL_UTILS_SRC)
	nasm -w+all -f bin -o micro_kernel.bin src/micro_kernel.asm

kernel.o: src/kernel.cpp
	g++ -Wall -Wextra -O2 -fno-exceptions -fno-rtti -ffreestanding -c src/kernel.cpp -o kernel.o

kernel2.bin:  kernel.o
	ld -e kernel_main -Ttext 0x10000 -o kernel.bin.o kernel.o
	ld -e kernel_main -i -Ttext 0x10000 -o kernel.bin.o kernel.o
	objcopy -R .note -R .comment -S -O binary kernel.bin.o kernel.bin

kernel.bin:  kernel.o
	g++ -T linker.ld -o kernel.bin.o -ffreestanding -O2 -nostdlib kernel.o
	objcopy -R .note -R .comment -S -O binary kernel.bin.o kernel.bin
	bash fill.bash

thor.flp: bootloader.bin micro_kernel.bin kernel.bin
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