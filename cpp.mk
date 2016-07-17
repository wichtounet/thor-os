CC=x86_64-elf-gcc
CXX=x86_64-elf-g++
AS=x86_64-elf-as
OC=x86_64-elf-objcopy
AR=x86_64-elf-ar

WARNING_FLAGS=-Wall -Wextra -pedantic -Wold-style-cast
COMMON_CPP_FLAGS=-masm=intel -I../../tstl/include/ -I../printf/include/ -I../tstl/include/ -I../tlib/include/ -Iinclude/ -nostdlib -g -Os -std=c++11 -fno-stack-protector -fno-exceptions -funsigned-char -fno-rtti -ffreestanding -fomit-frame-pointer -mno-red-zone -mno-3dnow -mno-mmx -fno-asynchronous-unwind-tables

DISABLE_SSE_FLAGS=-mno-sse -mno-sse2 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2
ENABLE_SSE_FLAGS=-msse -msse2 -msse3 -msse4 -msse4.1 -msse4.2

ENABLE_AVX_FLAGS=-mavx -mavx2
DISABLE_AVX_FLAGS=-mno-avx -mno-avx2

CPP_FLAGS_LOW=-march=i386 -m32 -fno-strict-aliasing -fno-pic -fno-toplevel-reorder $(DISABLE_SSE_FLAGS) $(DISABLE_AVX_FLAGS)

CPP_FLAGS_16=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mregparm=3 -mpreferred-stack-boundary=2
CPP_FLAGS_32=$(COMMON_CPP_FLAGS) $(CPP_FLAGS_LOW) -mpreferred-stack-boundary=4
CPP_FLAGS_64=$(COMMON_CPP_FLAGS) -mpreferred-stack-boundary=4 $(ENABLE_SSE_FLAGS) $(DISABLE_AVX_FLAGS)

KERNEL_CPP_FLAGS_64=$(CPP_FLAGS_64)

# Activate Stack Smashing Protection
KERNEL_CPP_FLAGS_64 += -fstack-protector

KERNEL_CPP_FLAGS_64 += -Iacpica/source/include

ACPICA_CPP_FLAGS = $(KERNEL_CPP_FLAGS_64) -include include/thor_acenv.hpp

COMMON_LINK_FLAGS=-lgcc

KERNEL_LINK_FLAGS=$(COMMON_LINK_FLAGS) -T linker.ld

LIB_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) -mcmodel=small -fPIC -ffunction-sections -fdata-sections
LIB_LINK_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) -mcmodel=small -fPIC -Wl,-gc-sections

PROGRAM_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) -I../../tlib/include/ -I../../printf/include/  -static -L../../tlib/ -ltlib -mcmodel=small -fPIC
PROGRAM_LINK_FLAGS=$(CPP_FLAGS_64) $(WARNING_FLAGS) $(COMMON_LINK_FLAGS) -static -L../../tlib/ -ltlib -mcmodel=small -fPIC -z max-page-size=0x1000 -T ../linker.ld -Wl,-gc-sections

D_FILES=

# Generate the rules for the CPP files of a directory
define compile_cpp_folder

$(1)/%.cpp.d: $(1)/%.cpp
	@ $(CXX) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -MM -MT $(1)/$$*.cpp.o $$< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $$@

$(1)/%.cpp.o: $(1)/%.cpp
	$(CXX) $(KERNEL_CPP_FLAGS_64) $(THOR_FLAGS) $(WARNING_FLAGS) -c $$< -o $(1)/$$*.cpp.o

folder_cpp_files=$(wildcard $(1)/*.cpp)
folder_d_files=$(folder_cpp_files:%.cpp=%.cpp.d)

D_FILES += $(folder_d_files)

endef

# Generate the rules for the APCICA C files of a components subdirectory
define acpica_folder_compile

acpica/source/components/$(1)/%.c.d: acpica/source/components/$(1)/%.c
	@ $(CXX) $(ACPICA_CPP_FLAGS) -include include/thor_acenv.hpp $(THOR_FLAGS) -MM -MT acpica/source/components/$(1)/$$*.c.o $$< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $$@

acpica/source/components/$(1)/%.c.o: acpica/source/components/$(1)/%.c
	$(CC) $(ACPICA_CPP_FLAGS) $(THOR_FLAGS) -c $$< -o $$@

folder_c_files=$(wildcard acpica/source/components/$(1)/*.c)
folder_d_files=$(folder_c_files:%.c=%.c.d)

endef
