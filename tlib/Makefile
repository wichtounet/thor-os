SHELL=/bin/bash

default: /tmp/ReOS/tlib/debug/libtlib.a

include ../cpp.mk

$(eval $(call tlib_compile_cpp_folder,src))

test:
	@ echo $(O_FILES)

/tmp/ReOS/tlib/debug/libtlib.a: $(O_FILES)
	@ mkdir -p /tmp/ReOS/tlib/debug/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link (tlib) $(FILE_COLOR)$@$(NO_COLOR)"
	@ ${AR} rcs /tmp/ReOS/tlib/debug/libtlib.a $(O_FILES)

-include $(D_FILES)

clean:
	@ echo -e "Remove compiled files (deps/objects)"
	@ rm -rf /tmp/ReOS/tlib/debug