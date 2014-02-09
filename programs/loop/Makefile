.PHONY: default clean

default: a.out

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) $(PROGRAM_FLAGS) $(CPP_FLAGS_64) $(WARNING_FLAGS) -c $< -o $@

a.out: main.cpp.o
	$(CC) $(PROGRAM_LINK_FLAGS) $(CPP_FLAGS_64) -o a.out main.cpp.o

clean:
	rm -f *.cpp.o
	rm -rf a.out
