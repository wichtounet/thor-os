.PHONY: default clean

default: shutdown

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

shutdown: main.cpp.o
	$(CXX) -o shutdown main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf shutdown
