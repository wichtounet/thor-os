default: libtlib.a

include ../cpp.mk

CPP_FILES=$(wildcard src/*.cpp)

D_FILES=$(CPP_FILES:%.cpp=%.cpp.d)

O_FILES=$(CPP_FILES:%.cpp=%.cpp.o)

src/%.cpp.d: $(CPP_FILES)
	@ $(CC) $(LIB_FLAGS) -MM -MT src/$*.cpp.o src/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

src/%.cpp.o:
	$(CC) $(LIB_FLAGS) -c $< -o $@

-include $(D_FILES)

libtlib.a: $(O_FILES)
	${AR} rcs libtlib.a ${O_FILES}

clean:
	rm -f libtlib.a
	rm -f $(O_FILES)
	rm -f $(D_FILES)
