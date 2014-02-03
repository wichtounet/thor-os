.PHONY: default clean

default: tsh

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) $(PROGRAM_FLAGS) $(CPP_FLAGS_64) $(WARNING_FLAGS) -c $< -o $@

tsh: main.cpp.o
	$(CC) $(PROGRAM_LINK_FLAGS) $(CPP_FLAGS_64) -o tsh main.cpp.o

clean:
	rm -f *.cpp.o
	rm -rf tsh
