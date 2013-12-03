default: kernel.bin

CC=x86_64-elf-g++
AS=x86_64-elf-as

THOR_FLAGS=-DCONFIG_HISTORY=y
WARNING_FLAGS=-Wall -Wextra -pedantic -Wold-style-cast -Wshadow
CPP_FLAGS=-masm=intel -Iinclude/ -nostdlib -Os -std=c++11 -fno-exceptions -fno-rtti -ffreestanding -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow $(WARNING_FLAGS)
KERNEL_FLAGS=$(CPP_FLAGS) $(THOR_FLAGS)
KERNEL_LINK_FLAGS=-lgcc -T linker.ld $(CPP_FLAGS)

KERNEL_CPP_FILES=$(wildcard src/*.cpp)
KERNEL_O_FILES=$(KERNEL_CPP_FILES:src/%.cpp=%.o)

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

LINK_O_FILES=crti.o $(CRTBEGIN_OBJ) $(KERNEL_O_FILES) $(CRTEND_OBJ) crtn.o

crti.o: src/crti.s
	$(AS) -c src/crti.s -o crti.o

crtn.o: src/crtn.s
	$(AS) -c src/crtn.s -o crtn.o

%.o : src/%.cpp
	$(CC) $(KERNEL_FLAGS) -c $< -o $@

kernel.bin: $(LINK_O_FILES)
	$(CC) $(KERNEL_LINK_FLAGS) -o kernel.bin.o $(LINK_O_FILES)
	objcopy -R .note -R .comment -S -O binary kernel.bin.o kernel.bin

clean:
	rm -f *.o
	rm -f *.bin
