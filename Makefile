default: thor.flp

KERNEL_SRC=$(wildcard src/*.asm)
KERNEL_UTILS_SRC=$(wildcard src/utils/*.asm)

bootloader.bin: src/bootloader/bootloader.asm
	nasm -w+all -f bin -o bootloader.bin src/bootloader/bootloader.asm

kernel.bin: $(KERNEL_SRC) $(KERNEL_UTILS_SRC)
	nasm -w+all -f bin -o kernel.bin src/kernel.asm

thor.flp: bootloader.bin kernel.bin
	cat bootloader.bin > thor.bin
	cat kernel.bin >> thor.bin
	dd status=noxfer conv=notrunc if=thor.bin of=thor.flp

qemu: thor.flp
	qemu-kvm -cpu host -fda thor.flp

bochs: thor.flp
	bochs -q -f bochsrc.txt

clean:
	rm -f *.bin
	rm -f *.flp