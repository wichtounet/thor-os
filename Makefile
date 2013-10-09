default: bootloader.flp

bootloader.bin: src/bootloader/bootloader.asm
	nasm -f bin -o bootloader.bin src/bootloader/bootloader.asm

bootloader.flp: bootloader.bin
	dd status=noxfer conv=notrunc if=bootloader.bin of=bootloader.flp

start: bootloader.flp
	qemu-kvm -fda bootloader.flp

clean:
	rm -f bootloader.bin
	rm -f bootloader.flp