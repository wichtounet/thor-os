default: kernel.bin

include ../cpp.mk

THOR_FLAGS=-DCONFIG_HISTORY=y

#TODO Generate also the o files coming from s files automatically, ignoring crti and crtn
O_FILES=boot_16_64.o boot_32_64.o src/isrs.s.o src/irqs.s.o src/arch.s.o src/syscalls.s.o src/task_switch.s.o

# Ask GCC for the crtbegin and crtend files
CRTBEGIN_OBJ:=$(shell $(CXX) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CXX) $(CFLAGS) -print-file-name=crtend.o)

# The final link files
SPECIAL_O_FILES=src/crti.s.o $(CRTBEGIN_OBJ) $(CRTEND_OBJ) src/crtn.s.o

# Compile the 16-bit and 32-bit parts of the executable

boot_16.o: src/boot/boot_16.cpp
	$(CXX) $(COMMON_CPP_FLAGS) $(FLAGS_16) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_16.cpp -o boot_16.o

boot_32.o: src/boot/boot_32.cpp
	$(CXX) $(COMMON_CPP_FLAGS) $(FLAGS_32) $(THOR_FLAGS) $(WARNING_FLAGS) -c src/boot/boot_32.cpp -o boot_32.o

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

-include $(D_FILES)

LINK_O_FILES=$(SPECIAL_O_FILES) $(O_FILES)

kernel.bin: $(LINK_O_FILES)
	$(CXX) $(KERNEL_LINK_FLAGS) $(KERNEL_CPP_FLAGS_64) -o kernel.bin.o $(LINK_O_FILES)
	$(OC) -R .note -R .comment -O binary --set-section-flags .bss=alloc,load,contents kernel.bin.o kernel.bin

clean:
	@ echo -e "Remove deps files"
	@ rm -f $(D_FILES)
	@ echo -e "Remove object files"
	@ rm -f $(O_FILES)
	@ rm -f crti.o
	@ rm -f crts.o
	@ echo -e "Remove bin files"
	@ rm -f *.bin
