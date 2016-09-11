SHELL=/bin/bash

default: all

.PHONY: all clean

/tmp/ReOS/bootloader/stage1.bin: stage1.asm
	@ mkdir -p /tmp/ReOS/bootloader/
	nasm -w+all -f bin -o /tmp/ReOS/bootloader/stage1.bin stage1.asm

/tmp/ReOS/bootloader/stage2.bin: stage2.asm
	@ mkdir -p /tmp/ReOS/bootloader/
	nasm -w+all -f bin -o /tmp/ReOS/bootloader/stage2.bin stage2.asm

all: /tmp/ReOS/bootloader/stage1.bin /tmp/ReOS/bootloader/stage2.bin

clean:
	rm -f /tmp/ReOS/bootloader/stage1.bin
	rm -f /tmp/ReOS/bootloader/stage2.bin