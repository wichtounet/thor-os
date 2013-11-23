default: stage1.bin stage2.bin padding.bin

stage1.bin: stage1.asm
	nasm -w+all -f bin -o stage1.bin stage1.asm

stage2.bin: stage2.asm
	nasm -w+all -f bin -o stage2.bin stage2.asm

padding.bin:
	dd if=/dev/zero of=padding.bin bs=512 count=34

clean:
	rm -f stage1.bin
	rm -f stage2.bin
