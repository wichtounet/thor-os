CC=x86_64-elf-g++
AS=x86_64-elf-as
OC=x86_64-elf-objcopy

WARNING_FLAGS=-Wall -Wextra -pedantic -Wold-style-cast -Wshadow
COMMON_CPP_FLAGS=-masm=intel -Iinclude/ -nostdlib -g -Os -std=c++11 -fno-stack-protector -fno-exceptions -funsigned-char -fno-rtti -ffreestanding -fomit-frame-pointer -mno-red-zone -mno-3dnow -mno-mmx -fno-asynchronous-unwind-tables

CPP_FLAGS_LOW=-march=i386 -m32 -fno-strict-aliasing -fno-pic -fno-toplevel-reorder -mno-sse -mno-sse2 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2

CPP_FLAGS_16=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mregparm=3 -mpreferred-stack-boundary=2
CPP_FLAGS_32=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mpreferred-stack-boundary=4
CPP_FLAGS_64=$(COMMON_CPP_FLAGS) -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2

COMMON_LINK_FLAGS=-lgcc
PROGRAM_LINK_FLAGS=-z max-page-size=0x1000 $(COMMON_LINK_FLAGS) -T ../linker.ld

PROGRAM_FLAGS=-I../../userlib/include/
