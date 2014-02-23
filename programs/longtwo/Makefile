.PHONY: default clean

default: longtwo

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) -c $< -o $@ $(PROGRAM_FLAGS)

longtwo: main.cpp.o
	$(CC) -o longtwo main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf longtwo
