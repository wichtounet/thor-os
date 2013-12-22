default: kernel.bin

CC=x86_64-elf-g++
AS=x86_64-elf-as
OC=x86_64-elf-objcopy

THOR_FLAGS=-DCONFIG_HISTORY=y

WARNING_FLAGS=-Wall -Wextra -pedantic -Wold-style-cast -Wshadow
COMMON_CPP_FLAGS=-masm=intel -Iinclude/ -nostdlib -Os -std=c++11 -fno-stack-protector -fno-exceptions -fno-rtti -ffreestanding -fomit-frame-pointer -mno-red-zone -mno-3dnow -mno-mmx

CPP_FLAGS_LOW=-march=i386 -m32 -fno-strict-aliasing -fno-pic -fno-toplevel-reorder -mno-sse -mno-sse2 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2

CPP_FLAGS_16=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mregparm=3 -mpreferred-stack-boundary=2
CPP_FLAGS_32=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mpreferred-stack-boundary=4
CPP_FLAGS_64=$(COMMON_CPP_FLAGS) -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2

LINK_FLAGS=-lgcc -T linker.ld

KERNEL_CPP_FILES=$(wildcard src/*.cpp)
KERNEL_CPP_STL_FILES=$(wildcard src/stl/*.cpp)

KERNEL_D_FILES=$(KERNEL_CPP_FILES:%.cpp=%.cpp.d)
KERNEL_D_STL_FILES=$(KERNEL_CPP_STL_FILES:%.cpp=%.cpp.d)

#TODO Generate also the o files coming from s files automatically, ignoring crti and crtn
KERNEL_O_FILES=boot_16_64.o boot_32_64.o $(KERNEL_CPP_FILES:%.cpp=%.cpp.o) $(KERNEL_CPP_STL_FILES:%.cpp=%.cpp.o) isrs.s.o irqs.s.o arch.s.o

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

LINK_O_FILES=crti.s.o $(CRTBEGIN_OBJ) $(KERNEL_O_FILES) $(CRTEND_OBJ) crtn.s.o

boot_16.o: src/boot/boot_16.cpp
	$(CC) $(CPP_FLAGS_16) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_16.cpp -o boot_16.o

boot_32.o: src/boot/boot_32.cpp
	$(CC) $(CPP_FLAGS_32) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_32.cpp -o boot_32.o

boot_16_64.o: boot_16.o
	$(OC) -I elf32-i386 -O elf64-x86-64 boot_16.o boot_16_64.o

boot_32_64.o: boot_32.o
	$(OC) -I elf32-i386 -O elf64-x86-64 boot_32.o boot_32_64.o

%.s.o: src/%.s
	$(AS) -c $< -o $@

src/%.cpp.d: $(KERNEL_CPP_FILES)
	@ $(CC) $(CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT src/$*.cpp.o src/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

src/stl/%.cpp.d: $(KERNEL_CPP_STL_FILES)
	@ $(CC) $(CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT src/stl/$*.cpp.o src/stl/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

src/%.cpp.o:
	$(CC) $(CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $< -o $@

src/stl/%.cpp.o:
	$(CC) $(CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $< -o $@

-include $(KERNEL_D_FILES)
-include $(KERNEL_D_STL_FILES)

kernel.bin: $(LINK_O_FILES)
	$(CC) $(LINK_FLAGS) $(CPP_FLAGS_64) -o kernel.bin.o $(LINK_O_FILES)
	$(OC) -R .note -R .comment -S -O binary kernel.bin.o kernel.bin

debug:
	echo $(KERNEL_CPP_STL_FILES)
	echo $(KERNEL_D_STL_FILES)

clean:
	rm -f $(KERNEL_D_FILES)
	rm -f $(KERNEL_D_STL_FILES)
	rm -f $(KERNEL_O_FILES)
	rm -f crti.o
	rm -f crts.o
	rm -f *.bin
