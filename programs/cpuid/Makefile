.PHONY: default clean

default: cpuid

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) -c $< -o $@ $(PROGRAM_FLAGS)

cpuid: main.cpp.o
	$(CC) -o cpuid main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf cpuid
