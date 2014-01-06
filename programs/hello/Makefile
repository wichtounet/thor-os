.PHONY: default clean

default: a.out

include ../../cpp.mk

%.cpp.o: src/%.cpp
	$(CC) $(CPP_FLAGS_64) $(WARNING_FLAGS) -c $< -o $@

a.out: main.cpp.o
	$(CC) $(COMMON_LINK_FLAGS) -e main $(CPP_FLAGS_64) -o a.out main.cpp.o

clean:
	rm *.cpp.o
	rm -rf a.out
