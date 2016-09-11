SHELL=/bin/bash

default: /tmp/ReOS/init/debug/init.bin

include ../cpp.mk

# Compile the 16-bit and 32-bit parts of the executable

INIT_FLAGS=-I../kernel/include

/tmp/ReOS/init/debug/boot_16.o: src/boot_16.cpp
	@ mkdir -p /tmp/ReOS/init/debug/
	@ $(CXX) $(COMMON_CPP_FLAGS) $(FLAGS_16) $(INIT_FLAGS) $(WARNING_FLAGS) -c src/boot_16.cpp -o /tmp/ReOS/init/debug/boot_16.o

/tmp/ReOS/init/debug/boot_32.o: src/boot_32.cpp
	@ mkdir -p /tmp/ReOS/init/debug/
	@ $(CXX) $(COMMON_CPP_FLAGS) $(FLAGS_32) $(INIT_FLAGS) $(WARNING_FLAGS) -c src/boot_32.cpp -o /tmp/ReOS/init/debug/boot_32.o

/tmp/ReOS/init/debug/boot_16_64.o: /tmp/ReOS/init/debug/boot_16.o
	@ $(OC) -I elf32-i386 -O elf64-x86-64 /tmp/ReOS/init/debug/boot_16.o /tmp/ReOS/init/debug/boot_16_64.o

/tmp/ReOS/init/debug/boot_32_64.o: /tmp/ReOS/init/debug/boot_32.o
	@ $(OC) -I elf32-i386 -O elf64-x86-64 /tmp/ReOS/init/debug/boot_32.o /tmp/ReOS/init/debug/boot_32_64.o

LINK_O_FILES=/tmp/ReOS/init/debug/boot_16_64.o /tmp/ReOS/init/debug/boot_32_64.o

/tmp/ReOS/init/debug/init.bin: $(LINK_O_FILES)
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link $(FILE_COLOR)$@$(NO_COLOR)"
	@ $(CXX) $(KERNEL_LINK_FLAGS) $(KERNEL_CPP_FLAGS_64) -o $@.o $(LINK_O_FILES)
	@ $(OC) -R .note -R .comment -O binary --set-section-flags .bss=alloc,load,contents $@.o $@

clean:
	@ echo -e "Remove compiled files (deps/objects)"
	@ rm -rf /tmp/ReOS/init/debug
