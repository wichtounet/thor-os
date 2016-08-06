default: debug/libtlib.a

include ../cpp.mk

$(eval $(call tlib_compile_cpp_folder,src))

debug/libtlib.a: $(O_FILES)
	@ mkdir -p debug/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link (tlib) $(FILE_COLOR)$@$(NO_COLOR)"
	@ ${AR} rcs debug/libtlib.a ${O_FILES}

-include $(D_FILES)

clean:
	@ echo -e "Remove compiled files (deps/objects)"
	@ rm -rf debug
