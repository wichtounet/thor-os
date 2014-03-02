.PHONY: default clean

default: uptime

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) -c $< -o $@ $(PROGRAM_FLAGS)

uptime: main.cpp.o
	$(CC) -o uptime main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf uptime
