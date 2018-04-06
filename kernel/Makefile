default: debug/kernel.bin

include ../cpp.mk

TESTSUITE_CPP_FILES=$(wildcard test_suite/*.cpp) src/path.cpp src/assert.cpp
TESTSUITE_FLAGS=--std=c++11 -I../tstl/include -I../tstl/test_suite/ -Iinclude -DTHOR_NO_ASSERT

TEST_CXX ?= g++

THOR_FLAGS=-DCONFIG_HISTORY=y

# Ask GCC for the crtbegin and crtend files
CRTBEGIN_OBJ:=$(shell $(CXX) $(KERNEL_CPP_FLAGS_64) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CXX) $(KERNEL_CPP_FLAGS_64) -print-file-name=crtend.o)

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

$(eval $(call compile_assembly_folder,src))

# Compile all the kernel CPP code

$(eval $(call compile_cpp_folder,src))
$(eval $(call compile_cpp_folder,src/net))
$(eval $(call compile_cpp_folder,src/conc))
$(eval $(call compile_cpp_folder,src/drivers))
$(eval $(call compile_cpp_folder,src/fs))
$(eval $(call compile_cpp_folder,src/vfs))

# Compile all the ACPICA C code

$(eval $(call acpica_folder_compile,dispatcher))
$(eval $(call acpica_folder_compile,events))
$(eval $(call acpica_folder_compile,executer))
$(eval $(call acpica_folder_compile,namespace))
$(eval $(call acpica_folder_compile,parser))
$(eval $(call acpica_folder_compile,hardware))
$(eval $(call acpica_folder_compile,tables))
$(eval $(call acpica_folder_compile,utilities))

-include $(D_FILES)

# Remove special files from O_FILES
O_FILES := $(filter-out debug/src/crti.s.o,$(O_FILES))
O_FILES := $(filter-out debug/src/crtn.s.o,$(O_FILES))

# COmpute the link files order
LINK_O_FILES=debug/src/crti.s.o $(CRTBEGIN_OBJ) $(O_FILES) $(CRTEND_OBJ) debug/src/crtn.s.o

debug/kernel.bin: $(LINK_O_FILES)
	@ mkdir -p debug/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link $(FILE_COLOR)$@$(NO_COLOR)"
	@ $(CXX) $(KERNEL_LINK_FLAGS) $(KERNEL_CPP_FLAGS_64) -o $@.o $(LINK_O_FILES)
	@ $(OC) -R .note -R .comment -O binary --set-section-flags .bss=alloc,load,contents $@.o $@

debug/bin/tester: $(TESTSUITE_CPP_FILES)
	@ mkdir -p debug/bin/
	$(TEST_CXX) $(WARNING_FLAGS) $(TESTSUITE_FLAGS) -o debug/bin/tester -g $(TESTSUITE_CPP_FILES)

test: debug/bin/tester
	./debug/bin/tester

clean:
	@ echo -e "Remove compiled files (deps/objects)"
	@ rm -rf debug
	@ rm -f crti.o
	@ rm -f crts.o
	@ echo -e "Remove bin files"
	@ rm -f *.bin
