default: debug/libtlib.a

include ../cpp.mk

# The files provided directly by the compiler
SPECIAL_O_FILES=$(CRTBEGIN_OBJ) $(CRTEND_OBJ)

$(eval $(call tlib_compile_cpp_folder,src))
$(eval $(call compile_assembly_folder,src))

# Compile the assembly code

debug/libtlib.a: $(SPECIAL_O_FILES) $(O_FILES)
	@ mkdir -p debug/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link (tlib) $(FILE_COLOR)$@$(NO_COLOR)"
	@ ${AR} rcs debug/libtlib.a $(SPECIAL_O_FILES) $(O_FILES)

-include $(D_FILES)

clean:
	@ echo -e "Remove compiled files (deps/objects)"
	@ rm -rf debug
