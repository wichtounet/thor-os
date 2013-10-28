MICRO_KERNEL_SRC=$(wildcard *.asm)
MICRO_KERNEL_UTILS_SRC=$(wildcard utils/*.asm)

micro_kernel.bin: $(MICRO_KERNEL_SRC) $(MICRO_KERNEL_UTILS_SRC)
	nasm -w+all -f bin -o micro_kernel.bin micro_kernel.asm
