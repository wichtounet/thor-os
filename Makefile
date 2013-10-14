default: thor.flp

bootloader.bin: src/bootloader/bootloader.asm
	nasm -w+all -f bin -o bootloader.bin src/bootloader/bootloader.asm

kernel.bin: src/kernel.asm
	nasm -w+all -f bin -o kernel.bin src/kernel.asm

thor.flp: bootloader.bin kernel.bin
	cat bootloader.bin > thor.bin
	cat kernel.bin >> thor.bin
	dd status=noxfer conv=notrunc if=thor.bin of=thor.flp

start: thor.flp
	qemu-kvm -cpu host -fda thor.flp

clean:
	rm -f *.bin
	rm -f *.flp