default: bootloader.bin

.PHONY: sectors clean

sectors:
	bash compute_sectors.bash

stage1.bin: stage1.asm
	nasm -w+all -f bin -o stage1.bin stage1.asm

stage2.bin: stage2.asm sectors.asm
	nasm -w+all -f bin -o stage2.bin stage2.asm

padding.bin:
	dd if=/dev/zero of=padding.bin bs=512 count=34

bootloader.bin: stage1.bin stage2.bin padding.bin
	cat stage1.bin > bootloader.bin
	cat stage2.bin >> bootloader.bin
	cat padding.bin >> bootloader.bin

clean:
	rm -f stage1.bin
	rm -f stage2.bin
	rm -f padding.bin
	rm -f bootloader.bin
