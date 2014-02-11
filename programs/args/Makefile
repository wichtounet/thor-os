.PHONY: default clean

default: args

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) -c $< -o $@ $(PROGRAM_FLAGS)

args: main.cpp.o
	$(CC) -o args main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf args
