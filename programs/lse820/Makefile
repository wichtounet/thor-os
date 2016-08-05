.PHONY: default clean

default: lse820

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

lse820: main.cpp.o
	$(CXX) -o lse820 main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf lspci
