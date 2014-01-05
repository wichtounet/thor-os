.PHONY: default clean

default: a.out

include ../../cpp.mk

a.out: src/main.cpp
	$(CC) $(CPP_FLAGS_64) $(WARNING_FLAGS) -c $< -o $@

clean:
	rm -rf a.out
