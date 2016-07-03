default: kernel.bin

include ../cpp.mk

THOR_FLAGS=-DCONFIG_HISTORY=y

KERNEL_CPP_FILES=$(wildcard src/*.cpp)
KERNEL_CPP_DRIVERS_FILES=$(wildcard src/drivers/*.cpp)
KERNEL_CPP_FS_FILES=$(wildcard src/fs/*.cpp)
KERNEL_CPP_VFS_FILES=$(wildcard src/vfs/*.cpp)

KERNEL_D_FILES=$(KERNEL_CPP_FILES:%.cpp=%.cpp.d)
KERNEL_D_DRIVERS_FILES=$(KERNEL_CPP_DRIVERS_FILES:%.cpp=%.cpp.d)
KERNEL_D_FS_FILES=$(KERNEL_CPP_FS_FILES:%.cpp=%.cpp.d)
KERNEL_D_VFS_FILES=$(KERNEL_CPP_VFS_FILES:%.cpp=%.cpp.d)

KERNEL_O_FILES=$(KERNEL_CPP_FILES:%.cpp=%.cpp.o)
KERNEL_O_DRIVERS_FILES=$(KERNEL_CPP_DRIVERS_FILES:%.cpp=%.cpp.o)
KERNEL_O_FS_FILES=$(KERNEL_CPP_FS_FILES:%.cpp=%.cpp.o)
KERNEL_O_VFS_FILES=$(KERNEL_CPP_VFS_FILES:%.cpp=%.cpp.o)

#TODO Generate also the o files coming from s files automatically, ignoring crti and crtn
O_FILES=boot_16_64.o boot_32_64.o $(KERNEL_O_FILES) $(KERNEL_O_FS_FILES) $(KERNEL_O_VFS_FILES) $(KERNEL_O_DRIVERS_FILES) src/isrs.s.o src/irqs.s.o src/arch.s.o src/syscalls.s.o src/task_switch.s.o

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

LINK_O_FILES=src/crti.s.o $(CRTBEGIN_OBJ) $(O_FILES) $(CRTEND_OBJ) src/crtn.s.o

boot_16.o: src/boot/boot_16.cpp
	$(CC) $(CPP_FLAGS_16) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_16.cpp -o boot_16.o

boot_32.o: src/boot/boot_32.cpp
	$(CC) $(CPP_FLAGS_32) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_32.cpp -o boot_32.o

boot_16_64.o: boot_16.o
	$(OC) -I elf32-i386 -O elf64-x86-64 boot_16.o boot_16_64.o

boot_32_64.o: boot_32.o
	$(OC) -I elf32-i386 -O elf64-x86-64 boot_32.o boot_32_64.o

src/%.s.o: src/%.s
	$(AS) -g -c $< -o $@

src/%.cpp.d: $(KERNEL_CPP_FILES)
	@ $(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT src/$*.cpp.o src/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

src/drivers/%.cpp.d: $(KERNEL_CPP_DRIVERS_FILES)
	@ $(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT src/drivers/$*.cpp.o src/drivers/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

src/fs/%.cpp.d: $(KERNEL_CPP_FS_FILES)
	@ $(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT src/fs/$*.cpp.o src/fs/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

src/vfs/%.cpp.d: $(KERNEL_CPP_VFS_FILES)
	@ $(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT src/vfs/$*.cpp.o src/vfs/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

src/%.cpp.o:
	$(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $< -o $@

src/drivers/%.cpp.o:
	$(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $< -o $@

src/vfs/%.cpp.o:
	$(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $< -o $@

src/fs/%.cpp.o:
	$(CC) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $< -o $@

-include $(KERNEL_D_FILES)
-include $(KERNEL_D_DRIVERS_FILES)
-include $(KERNEL_D_VFS_FILES)
-include $(KERNEL_D_FS_FILES)

kernel.bin: $(LINK_O_FILES)
	$(CC) $(KERNEL_LINK_FLAGS) $(KERNEL_CPP_FLAGS_64) -o kernel.bin.o $(LINK_O_FILES)
	$(OC) -R .note -R .comment -O binary --set-section-flags .bss=alloc,load,contents kernel.bin.o kernel.bin

clean:
	rm -f $(KERNEL_D_FILES)
	rm -f $(KERNEL_D_DRIVERS_FILES)
	rm -f $(KERNEL_D_FS_FILES)
	rm -f $(KERNEL_D_VFS_FILES)
	rm -f $(O_FILES)
	rm -f crti.o
	rm -f crts.o
	rm -f *.bin
