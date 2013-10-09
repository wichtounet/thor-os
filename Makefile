default: thor.flp

thor.bin: src/thor.asm
	nasm -f bin -o thor.bin src/thor.asm

thor.flp: thor.bin
	dd status=noxfer conv=notrunc if=thor.bin of=thor.flp

start: thor.flp
	qemu-kvm -fda thor.flp

clean:
	rm -f thor.bin
	rm -f thor.flp