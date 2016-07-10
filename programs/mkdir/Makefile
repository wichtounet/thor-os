.PHONY: default clean

default: mkdir

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

mkdir: main.cpp.o
	$(CXX) -o mkdir main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf mkdir
