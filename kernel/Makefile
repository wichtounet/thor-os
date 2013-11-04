CC=x86_64-elf-g++

WARNING_FLAGS=-Wall -Wextra -pedantic -Wold-style-cast -Wshadow
CPP_FLAGS=-masm=intel -Iinclude/ -nostdlib -O1 -std=c++11 -fno-exceptions -fno-rtti -ffreestanding -lgcc -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow $(WARNING_FLAGS)
KERNEL_FLAGS=$(CPP_FLAGS)
KERNEL_LINK_FLAGS=-T linker.ld $(CPP_FLAGS)

KERNEL_O_FILES=kernel.o keyboard.o console.o kernel_utils.o timer.o shell.o utils.o memory.o ata.o thor.o

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

LINK_O_FILES=$(KERNEL_O_FILES)


kernel.bin: $(LINK_O_FILES)
	$(CC) $(KERNEL_LINK_FLAGS) -o kernel.bin.o $(LINK_O_FILES)
	objcopy -R .note -R .comment -S -O binary kernel.bin.o kernel.bin

kernel.o: src/kernel.cpp
	$(CC) $(KERNEL_FLAGS) -c src/kernel.cpp -o kernel.o

keyboard.o: src/keyboard.cpp
	$(CC) $(KERNEL_FLAGS) -c src/keyboard.cpp -o keyboard.o

console.o: src/console.cpp
	$(CC) $(KERNEL_FLAGS) -c src/console.cpp -o console.o

kernel_utils.o: src/kernel_utils.cpp
	$(CC) $(KERNEL_FLAGS) -c src/kernel_utils.cpp -o kernel_utils.o

timer.o: src/timer.cpp
	$(CC) $(KERNEL_FLAGS) -c src/timer.cpp -o timer.o

shell.o: src/shell.cpp
	$(CC) $(KERNEL_FLAGS) -c src/shell.cpp -o shell.o

utils.o: src/utils.cpp
	$(CC) $(KERNEL_FLAGS) -c src/utils.cpp -o utils.o

memory.o: src/memory.cpp
	$(CC) $(KERNEL_FLAGS) -c src/memory.cpp -o memory.o

ata.o: src/ata.cpp
	$(CC) $(KERNEL_FLAGS) -c src/ata.cpp -o ata.o

thor.o: src/thor.cpp
	$(CC) $(KERNEL_FLAGS) -c src/thor.cpp -o thor.o

clean:
	rm -f *.o
	rm -f *.bin
