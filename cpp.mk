CC=x86_64-elf-gcc
CXX=x86_64-elf-g++
AS=x86_64-elf-as
OC=x86_64-elf-objcopy
AR=x86_64-elf-ar

WARNING_FLAGS=-Wall -Wextra -pedantic -Wold-style-cast
COMMON_C_FLAGS=-masm=intel -I../../tstl/include/ -I../printf/include/ -I../tstl/include/ -I../tlib/include/ -Iinclude/ -nostdlib -g -Os -fno-stack-protector -fno-exceptions -funsigned-char -ffreestanding -fomit-frame-pointer -mno-red-zone -mno-3dnow -mno-mmx -fno-asynchronous-unwind-tables

# Include ACPICA
COMMON_C_FLAGS += -isystem acpica/source/include

# Add more flags for C++
COMMON_CPP_FLAGS=$(COMMON_C_FLAGS) -std=c++11 -fno-rtti

DISABLE_SSE_FLAGS=-mno-sse -mno-sse2 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2
ENABLE_SSE_FLAGS=-msse -msse2 -msse3 -msse4 -msse4.1 -msse4.2

ENABLE_AVX_FLAGS=-mavx -mavx2
DISABLE_AVX_FLAGS=-mno-avx -mno-avx2

CPP_FLAGS_LOW=-march=i386 -m32 -fno-strict-aliasing -fno-pic -fno-toplevel-reorder $(DISABLE_SSE_FLAGS) $(DISABLE_AVX_FLAGS)

FLAGS_16=$(CPP_FLAGS_LOW) -mregparm=3 -mpreferred-stack-boundary=2
FLAGS_32=$(CPP_FLAGS_LOW) -mpreferred-stack-boundary=4
FLAGS_64=-mpreferred-stack-boundary=4 $(ENABLE_SSE_FLAGS) $(DISABLE_AVX_FLAGS)

# Activate Stack Smashing Protection
FLAGS_64 += -fstack-protector

KERNEL_CPP_FLAGS_64=$(COMMON_CPP_FLAGS) $(FLAGS_64)

ACPICA_C_FLAGS= $(COMMON_C_FLAGS) $(FLAGS_64) -include include/thor_acenv.hpp -include include/thor_acenvex.hpp

COMMON_LINK_FLAGS=-lgcc

KERNEL_LINK_FLAGS=$(COMMON_LINK_FLAGS) -T linker.ld

LIB_FLAGS=$(COMMON_CPP_FLAGS) $(FLAGS_64) $(WARNING_FLAGS) -mcmodel=small -fPIC -ffunction-sections -fdata-sections
LIB_LINK_FLAGS=$(COMMON_CPP_FLAGS) $(FLAGS_64) $(WARNING_FLAGS) -mcmodel=small -fPIC -Wl,-gc-sections

PROGRAM_FLAGS=$(COMMON_CPP_FLAGS) $(FLAGS_64) $(WARNING_FLAGS) -I../../tlib/include/ -I../../printf/include/  -static -L../../tlib/debug/ -ltlib -mcmodel=small -fPIC
PROGRAM_LINK_FLAGS=$(COMMON_CPP_FLAGS) $(FLAGS_64) $(WARNING_FLAGS) $(COMMON_LINK_FLAGS) -static -L../../tlib/debug/ -ltlib -mcmodel=small -fPIC -z max-page-size=0x1000 -T ../linker.ld -Wl,-gc-sections

# Ask GCC for the crtbegin and crtend files
CRTBEGIN_OBJ:=$(shell $(CXX) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CXX) -print-file-name=crtend.o)

NO_COLOR=\x1b[0m
MODE_COLOR=\x1b[31;01m
FILE_COLOR=\x1b[35;01m

# Generate the rules for the CPP files of a directory
define compile_cpp_folder

debug/$(1)/%.cpp.d: $(1)/%.cpp
	@ mkdir -p debug/$(1)/
	@ $(CXX) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT debug/$(1)/$$*.cpp.o $$< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $$@

debug/$(1)/%.cpp.o: $(1)/%.cpp
	@ mkdir -p debug/$(1)/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Compile $(FILE_COLOR)$(1)/$$*.cpp$(NO_COLOR)"
	@ $(CXX) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $$< -o $$@

folder_cpp_files := $(wildcard $(1)/*.cpp)
folder_d_files   := $$(folder_cpp_files:%.cpp=debug/%.cpp.d)
folder_o_files   := $$(folder_cpp_files:%.cpp=debug/%.cpp.o)

D_FILES := $(D_FILES) $$(folder_d_files)
O_FILES := $(O_FILES) $$(folder_o_files)

endef

# Generate the rules for the APCICA C files of a components subdirectory
define acpica_folder_compile

debug/acpica/source/components/$(1)/%.c.d: acpica/source/components/$(1)/%.c
	@ mkdir -p debug/acpica/source/components/$(1)/
	@ $(CXX) $(ACPICA_C_FLAGS) $(THOR_FLAGS) -MM -MT acpica/source/components/$(1)/$$*.c.o $$< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $$@

debug/acpica/source/components/$(1)/%.c.o: acpica/source/components/$(1)/%.c
	@ mkdir -p debug/acpica/source/components/$(1)/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Compile (ACPICA) $(FILE_COLOR)$(1)/$$*.cpp$(NO_COLOR)"
	@ $(CC) $(ACPICA_C_FLAGS) $(THOR_FLAGS) -c $$< -o $$@

acpica_folder_c_files := $(wildcard acpica/source/components/$(1)/*.c)
acpica_folder_d_files := $$(acpica_folder_c_files:%.c=debug/%.c.d)
acpica_folder_o_files := $$(acpica_folder_c_files:%.c=debug/%.c.o)

D_FILES := $(D_FILES) $$(acpica_folder_d_files)
O_FILES := $(O_FILES) $$(acpica_folder_o_files)

endef

# Generate the rules for the CPP files of a directory
define program_compile_cpp_folder

debug/$(1)/%.cpp.o: $(1)/%.cpp
	@ mkdir -p debug/$(1)/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Compile (program) $(FILE_COLOR)$(1)/$$*.cpp$(NO_COLOR)"
	@ $(CXX) -c $$< -o $$@ $(PROGRAM_FLAGS)

folder_cpp_files := $(wildcard $(1)/*.cpp)
folder_o_files   := $$(folder_cpp_files:%.cpp=debug/%.cpp.o)

O_FILES := $(O_FILES) $$(folder_o_files)

endef

define program_link_executable

debug/$(1): $(O_FILES)
	@ mkdir -p debug/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link (program) $(FILE_COLOR)$$@$(NO_COLOR)"
	@ $(CXX) -o debug/$(1) $(O_FILES) $(PROGRAM_LINK_FLAGS)

link: debug/$(1)

endef

# Generate the rules for the CPP files of a directory
define tlib_compile_cpp_folder

debug/$(1)/%.cpp.d: $(1)/%.cpp
	@ mkdir -p debug/$(1)/
	@ $(CXX) $(LIB_FLAGS) -MM -MT debug/$(1)/$$*.cpp.o $(1)/$$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $$@

debug/$(1)/%.cpp.o: $(1)/%.cpp
	@ mkdir -p debug/$(1)/
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Compile (tlib) $(FILE_COLOR)$(1)/$$*.cpp$(NO_COLOR)"
	@ $(CXX) $(LIB_FLAGS) -c $$< -o $$@

folder_cpp_files := $(wildcard $(1)/*.cpp)
folder_d_files   := $$(folder_cpp_files:%.cpp=debug/%.cpp.d)
folder_o_files   := $$(folder_cpp_files:%.cpp=debug/%.cpp.o)

D_FILES := $(D_FILES) $$(folder_d_files)
O_FILES := $(O_FILES) $$(folder_o_files)

endef
