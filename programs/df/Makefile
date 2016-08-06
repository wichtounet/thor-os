.PHONY: default clean

EXEC_NAME=df

default: link

include ../../cpp.mk

$(eval $(call program_compile_cpp_folder,src))
$(eval $(call program_link_executable,$(EXEC_NAME)))

clean:
	@ echo -e "Remove compiled files"
	@ rm -rf debug
