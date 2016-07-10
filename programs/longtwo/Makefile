.PHONY: default clean

default: longtwo

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

longtwo: main.cpp.o
	$(CXX) -o longtwo main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -f *.cpp.o
	rm -rf longtwo
