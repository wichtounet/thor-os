.PHONY: default clean

default: mkdir

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) -c $< -o $@ $(PROGRAM_FLAGS)

mkdir: main.cpp.o
	$(CC) -o mkdir main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf mkdir
