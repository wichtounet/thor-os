default: /tmp/ReOS/kernel/debug/kernel.bin

include ../cpp.mk

THOR_FLAGS=-DCONFIG_HISTORY=y

#TODO Generate also the o files coming from s files automatically, ignoring crti and crtn
O_FILES=/tmp/ReOS/kernel/src/isrs.s.o /tmp/ReOS/kernel/src/irqs.s.o /tmp/ReOS/kernel/src/arch.s.o /tmp/ReOS/kernel/src/syscalls.s.o /tmp/ReOS/kernel/src/task_switch.s.o

# Ask GCC for the crtbegin and crtend files
CRTBEGIN_OBJ:=$(shell $(CXX) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CXX) $(CFLAGS) -print-file-name=crtend.o)

# The final link files
SPECIAL_O_FILES=/tmp/ReOS/kernel/src/crti.s.o $(CRTBEGIN_OBJ) $(CRTEND_OBJ) /tmp/ReOS/kernel/src/crtn.s.o

# Compile the assembly code

/tmp/ReOS/kernel/src/%.s.o: src/%.s
	@ mkdir -p /tmp/ReOS/kernel/src/
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
$(eval $(call acpica_folder_compile,parser))
$(eval $(call acpica_folder_compile,hardware))
$(eval $(call acpica_folder_compile,tables))
$(eval $(call acpica_folder_compile,utilities))

-include $(D_FILES)

LINK_O_FILES=$(SPECIAL_O_FILES) $(O_FILES)

/tmp/ReOS/kernel/debug/kernel.bin: $(LINK_O_FILES)
	@ mkdir -p /tmp/ReOS/kernel/debug/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link $(FILE_COLOR)$@$(NO_COLOR)"
	@ $(CXX) $(KERNEL_LINK_FLAGS) $(KERNEL_CPP_FLAGS_64) -o $@.o $(LINK_O_FILES)
	@ $(OC) -R .note -R .comment -O binary --set-section-flags .bss=alloc,load,contents $@.o $@

clean:
	@ echo -e "Remove compiled files (deps/objects)"
	@ rm -rf /tmp/ReOS/kernel/debug
	@ rm -f /tmp/ReOS/kernel/crti.o
	@ rm -f /tmp/ReOS/kernel/crts.o
	@ echo -e "Remove bin files"
	@ rm -f /tmp/ReOS/kernel/*.bin
