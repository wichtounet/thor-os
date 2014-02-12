.PHONY: default clean

default: stat

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) -c $< -o $@ $(PROGRAM_FLAGS)

stat: main.cpp.o
	$(CC) -o stat main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf stat
