.PHONY: default clean

default: keyboard

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

keyboard: main.cpp.o
	$(CXX) -o keyboard main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf keyboard
