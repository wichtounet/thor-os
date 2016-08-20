default: debug/libtlib.a

include ../cpp.mk

$(eval $(call tlib_compile_cpp_folder,src))
$(eval $(call compile_assembly_folder,src))

# Compile the assembly code

O_FILES := $(filter-out debug/src/crti.s.o,$(O_FILES))
O_FILES := $(filter-out debug/src/crtn.s.o,$(O_FILES))

debug/libtlib.a: debug/src/crti.s.o debug/src/crtn.s.o $(O_FILES)
	@ mkdir -p debug/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link (tlib) $(FILE_COLOR)$@$(NO_COLOR)"
	@ ${AR} rcs debug/libtlib.a $(O_FILES)

-include $(D_FILES)

clean:
	@ echo -e "Remove compiled files (deps/objects)"
	@ rm -rf debug
