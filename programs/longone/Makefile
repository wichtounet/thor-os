.PHONY: default clean

default: longone

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

longone: main.cpp.o
	$(CXX) -o longone main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf longone
