CC=x86_64-elf-g++
AS=x86_64-elf-as
OC=x86_64-elf-objcopy
AR=x86_64-elf-ar

WARNING_FLAGS=-Wall -Wextra -pedantic -Wold-style-cast
COMMON_CPP_FLAGS=-masm=intel -I../../tstl/include/ -I../tstl/include/ -I../tlib/include/ -Iinclude/ -nostdlib -g -Os -std=c++11 -fno-stack-protector -fno-exceptions -funsigned-char -fno-rtti -ffreestanding -fomit-frame-pointer -mno-red-zone -mno-3dnow -mno-mmx -fno-asynchronous-unwind-tables

CPP_FLAGS_LOW=-march=i386 -m32 -fno-strict-aliasing -fno-pic -fno-toplevel-reorder -mno-sse -mno-sse2 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2

CPP_FLAGS_16=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mregparm=3 -mpreferred-stack-boundary=2
CPP_FLAGS_32=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mpreferred-stack-boundary=4
CPP_FLAGS_64=$(COMMON_CPP_FLAGS) -mpreferred-stack-boundary=4 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2 -mno-avx

COMMON_LINK_FLAGS=-lgcc

KERNEL_LINK_FLAGS=$(COMMON_LINK_FLAGS) -T linker.ld

LIB_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) -mcmodel=small -fPIC -ffunction-sections -fdata-sections
LIB_LINK_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) -mcmodel=small -fPIC -Wl,-gc-sections

PROGRAM_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) -I../../tlib/include/ -static -L../../tlib/ -ltlib -mcmodel=small -fPIC
PROGRAM_LINK_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) $(COMMON_LINK_FLAGS) -static -L../../tlib/ -ltlib -mcmodel=small -fPIC -z max-page-size=0x1000 -T ../linker.ld -Wl,-gc-sections
