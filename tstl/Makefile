default: debug/bin/tester

include ../cpp.mk

TESTSUITE_CPP_FILES=$(wildcard test_suite/*.cpp)

debug/bin/tester: $(TESTSUITE_CPP_FILES)
	@ mkdir -p debug/bin/
	g++ $(WARNING_FLAGS) --std=c++11 -Iinclude -o debug/bin/tester -g $(TESTSUITE_CPP_FILES)

test: debug/bin/tester
	./debug/bin/tester

clean:
	rm -rf debug
