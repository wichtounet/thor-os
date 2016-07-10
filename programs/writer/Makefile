.PHONY: default clean

default: writer

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

writer: main.cpp.o
	$(CXX) -o writer main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf writer
