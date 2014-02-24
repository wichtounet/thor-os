.PHONY: default clean

default: readelf

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) -c $< -o $@ $(PROGRAM_FLAGS)

readelf: main.cpp.o
	$(CC) -o readelf main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf readelf
