default: kernel.bin

include ../cpp.mk

THOR_FLAGS=-DCONFIG_HISTORY=y

KERNEL_CPP_FILES=$(wildcard src/*.cpp)
KERNEL_CPP_DRIVERS_FILES=$(wildcard src/drivers/*.cpp)
KERNEL_CPP_FS_FILES=$(wildcard src/fs/*.cpp)
KERNEL_CPP_VFS_FILES=$(wildcard src/vfs/*.cpp)

ACPICA_DISPATCHER_CPP_FILES=$(wildcard acpica/source/components/dispatcher/*.c)
ACPICA_EVENTS_CPP_FILES=$(wildcard acpica/source/components/events/*.c)
ACPICA_EXECUTER_CPP_FILES=$(wildcard acpica/source/components/executer/*.c)
ACPICA_NAMESPACE_CPP_FILES=$(wildcard acpica/source/components/namespace/*.c)
ACPICA_RESOURCES_CPP_FILES=$(wildcard acpica/source/components/resources/*.c)
ACPICA_TABLES_CPP_FILES=$(wildcard acpica/source/components/tables/*.c)
ACPICA_UTILITIES_CPP_FILES=$(wildcard acpica/source/components/utilities/*.c)

KERNEL_D_FILES=$(KERNEL_CPP_FILES:%.cpp=%.cpp.d)
KERNEL_D_DRIVERS_FILES=$(KERNEL_CPP_DRIVERS_FILES:%.cpp=%.cpp.d)
KERNEL_D_FS_FILES=$(KERNEL_CPP_FS_FILES:%.cpp=%.cpp.d)
KERNEL_D_VFS_FILES=$(KERNEL_CPP_VFS_FILES:%.cpp=%.cpp.d)

ACPICA_DISPATCHER_D_FILES=$(ACPICA_DISPATCHER_CPP_FILES:%.c=%.c.d)
ACPICA_EVENTS_D_FILES=$(ACPICA_EVENTS_CPP_FILES:%.c=%.c.d)
ACPICA_EXECUTER_D_FILES=$(ACPICA_EXECUTER_CPP_FILES:%.c=%.c.d)
ACPICA_NAMESPACE_D_FILES=$(ACPICA_NAMESPACE_CPP_FILES:%.c=%.c.d)
ACPICA_RESOURCES_D_FILES=$(ACPICA_RESOURCES_CPP_FILES:%.c=%.c.d)
ACPICA_TABLES_D_FILES=$(ACPICA_TABLES_CPP_FILES:%.c=%.c.d)
ACPICA_UTILITIES_D_FILES=$(ACPICA_UTILITIES_CPP_FILES:%.c=%.c.d)

KERNEL_O_FILES=$(KERNEL_CPP_FILES:%.cpp=%.cpp.o)
KERNEL_O_DRIVERS_FILES=$(KERNEL_CPP_DRIVERS_FILES:%.cpp=%.cpp.o)
KERNEL_O_FS_FILES=$(KERNEL_CPP_FS_FILES:%.cpp=%.cpp.o)
KERNEL_O_VFS_FILES=$(KERNEL_CPP_VFS_FILES:%.cpp=%.cpp.o)

ACPICA_DISPATCHER_O_FILES=$(ACPICA_DISPATCHER_CPP_FILES:%.c=%.c.o)
ACPICA_EVENTS_O_FILES=$(ACPICA_EVENTS_CPP_FILES:%.c=%.c.o)
ACPICA_EXECUTER_O_FILES=$(ACPICA_EXECUTER_CPP_FILES:%.c=%.c.o)
ACPICA_NAMESPACE_O_FILES=$(ACPICA_NAMESPACE_CPP_FILES:%.c=%.c.o)
ACPICA_RESOURCES_O_FILES=$(ACPICA_RESOURCES_CPP_FILES:%.c=%.c.o)
ACPICA_TABLES_O_FILES=$(ACPICA_TABLES_CPP_FILES:%.c=%.c.o)
ACPICA_UTILITIES_O_FILES=$(ACPICA_UTILITIES_CPP_FILES:%.c=%.c.o)
ACPICA_O_FILES=$(ACPICA_DISPATCHER_O_FILES) $(ACPICA_EVENTS_O_FILES) $(ACPICA_EXECUTER_O_FILES) $(ACPICA_NAMESPACE_O_FILES) $(ACPICA_RESOURCES_O_FILES) $(ACPICA_TABLES_O_FILES) $(ACPICA_UTILITIES_O_FILES)

#TODO Generate also the o files coming from s files automatically, ignoring crti and crtn
O_FILES=boot_16_64.o boot_32_64.o $(KERNEL_O_FILES) $(KERNEL_O_FS_FILES) $(KERNEL_O_VFS_FILES) $(KERNEL_O_DRIVERS_FILES) src/isrs.s.o src/irqs.s.o src/arch.s.o src/syscalls.s.o src/task_switch.s.o

# Add the APCICA files that are necessary to compile
O_FILES += $(ACPICA_O_FILES)

# Ask GCC for the crtbegin and crtend files
CRTBEGIN_OBJ:=$(shell $(CXX) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CXX) $(CFLAGS) -print-file-name=crtend.o)

# The final link files
LINK_O_FILES=src/crti.s.o $(CRTBEGIN_OBJ) $(O_FILES) $(CRTEND_OBJ) src/crtn.s.o

# Compile the 16-bit and 32-bit parts of the executable

boot_16.o: src/boot/boot_16.cpp
	$(CXX) $(CPP_FLAGS_16) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_16.cpp -o boot_16.o

boot_32.o: src/boot/boot_32.cpp
	$(CXX) $(CPP_FLAGS_32) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_32.cpp -o boot_32.o

boot_16_64.o: boot_16.o
	$(OC) -I elf32-i386 -O elf64-x86-64 boot_16.o boot_16_64.o

boot_32_64.o: boot_32.o
	$(OC) -I elf32-i386 -O elf64-x86-64 boot_32.o boot_32_64.o

# Compile the assembly code

src/%.s.o: src/%.s
	$(AS) -g -c $< -o $@

# Compile all the kernel CPP code

$(eval $(call compile_cpp_folder,src))
$(eval $(call compile_cpp_folder,src/drivers))
$(eval $(call compile_cpp_folder,src/fs))
$(eval $(call compile_cpp_folder,src/vfs))

# Compile all the ACPICA C code

$(eval $(call acpica_folder_compile,dispatcher))
$(eval $(call acpica_folder_compile,events))
$(eval $(call acpica_folder_compile,executer))
$(eval $(call acpica_folder_compile,namespace))
$(eval $(call acpica_folder_compile,resources))
$(eval $(call acpica_folder_compile,tables))
$(eval $(call acpica_folder_compile,utilities))

-include $(KERNEL_D_FILES)
-include $(KERNEL_D_DRIVERS_FILES)
-include $(KERNEL_D_VFS_FILES)
-include $(KERNEL_D_FS_FILES)
-include $(ACPICA_DISPATCHER_D_FILES)
-include $(ACPICA_EVENTS_D_FILES)
-include $(ACPICA_EXECUTER_D_FILES)
-include $(ACPICA_NAMESPACE_D_FILES)
-include $(ACPICA_RESOURCES_D_FILES)
-include $(ACPICA_TABLES_D_FILES)
-include $(ACPICA_UTILITIES_D_FILES)

kernel.bin: $(LINK_O_FILES)
	$(CXX) $(KERNEL_LINK_FLAGS) $(KERNEL_CPP_FLAGS_64) -o kernel.bin.o $(LINK_O_FILES)
	$(OC) -R .note -R .comment -O binary --set-section-flags .bss=alloc,load,contents kernel.bin.o kernel.bin

clean:
	rm -f $(KERNEL_D_FILES)
	rm -f $(KERNEL_D_DRIVERS_FILES)
	rm -f $(KERNEL_D_FS_FILES)
	rm -f $(KERNEL_D_VFS_FILES)
	rm -f $(ACPICA_DISPATCHER_D_FILES)
	rm -f $(ACPICA_EVENTS_D_FILES)
	rm -f $(ACPICA_EXECUTER_D_FILES)
	rm -f $(ACPICA_NAMESPACE_D_FILES)
	rm -f $(ACPICA_RESOURCES_D_FILES)
	rm -f $(ACPICA_TABLES_D_FILES)
	rm -f $(ACPICA_UTILITIES_D_FILES)
	rm -f $(O_FILES)
	rm -f crti.o
	rm -f crts.o
	rm -f *.bin
