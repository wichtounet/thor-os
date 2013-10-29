KERNEL_FLAGS=-masm=intel -Iinclude/ -O1 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti -ffreestanding
KERNEL_LINK_FLAGS=-std=c++11 -T linker.ld -ffreestanding -O1 -nostdlib

KERNEL_O_FILES=kernel.o keyboard.o console.o kernel_utils.o timer.o shell.o utils.o

kernel.bin: $(KERNEL_O_FILES)
	g++ $(KERNEL_LINK_FLAGS) -o kernel.bin.o $(KERNEL_O_FILES)
	objcopy -R .note -R .comment -S -O binary kernel.bin.o kernel.bin

kernel.o: src/kernel.cpp
	g++ $(KERNEL_FLAGS) -c src/kernel.cpp -o kernel.o

keyboard.o: src/keyboard.cpp
	g++ $(KERNEL_FLAGS) -c src/keyboard.cpp -o keyboard.o

console.o: src/console.cpp
	g++ $(KERNEL_FLAGS) -c src/console.cpp -o console.o

kernel_utils.o: src/kernel_utils.cpp
	g++ $(KERNEL_FLAGS) -c src/kernel_utils.cpp -o kernel_utils.o

timer.o: src/timer.cpp
	g++ $(KERNEL_FLAGS) -c src/timer.cpp -o timer.o

shell.o: src/shell.cpp
	g++ $(KERNEL_FLAGS) -c src/shell.cpp -o shell.o

utils.o: src/utils.cpp
	g++ $(KERNEL_FLAGS) -c src/utils.cpp -o utils.o

clean:
	rm -f *.o
	rm -f *.bin
