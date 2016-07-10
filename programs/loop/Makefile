.PHONY: default clean

default: loop

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CXX) $(PROGRAM_FLAGS) $(CPP_FLAGS_64) $(WARNING_FLAGS) -c $< -o $@

loop: main.cpp.o
	$(CXX) $(PROGRAM_LINK_FLAGS) $(CPP_FLAGS_64) -o loop main.cpp.o

clean:
	rm -f *.cpp.o
	rm -rf loop
