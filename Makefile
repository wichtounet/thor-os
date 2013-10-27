default: thor.flp

bootloader.bin: bootloader/bootloader.asm
	nasm -w+all -f bin -o bootloader.bin bootloader/bootloader.asm

MICRO_KERNEL_SRC=$(wildcard micro_kernel/*.asm)
MICRO_KERNEL_UTILS_SRC=$(wildcard micro_kernel/utils/*.asm)

micro_kernel.bin: $(MICRO_KERNEL_SRC) $(MICRO_KERNEL_UTILS_SRC)
	nasm -w+all -f bin -o micro_kernel.bin micro_kernel/micro_kernel.asm

KERNEL_FLAGS=-masm=intel -Ikernel/include/ -O1 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti -ffreestanding
KERNEL_LINK_FLAGS=-std=c++11 -T linker.ld -ffreestanding -O1 -nostdlib

KERNEL_O_FILES=kernel.o keyboard.o console.o kernel_utils.o

kernel.o: kernel/src/kernel.cpp
	g++ $(KERNEL_FLAGS) -c kernel/src/kernel.cpp -o kernel.o

keyboard.o: kernel/src/keyboard.cpp
	g++ $(KERNEL_FLAGS) -c kernel/src/keyboard.cpp -o keyboard.o

console.o: kernel/src/console.cpp
	g++ $(KERNEL_FLAGS) -c kernel/src/console.cpp -o console.o

kernel_utils.o: kernel/src/kernel_utils.cpp
	g++ $(KERNEL_FLAGS) -c kernel/src/kernel_utils.cpp -o kernel_utils.o

kernel.bin: $(KERNEL_O_FILES)
	g++ $(KERNEL_LINK_FLAGS) -o kernel.bin.o $(KERNEL_O_FILES)
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