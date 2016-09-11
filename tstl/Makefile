default: tester

include ../cpp.mk

TESTSUITE_CPP_FILES=$(wildcard test_suite/*.cpp)

tester: $(TESTSUITE_CPP_FILES)
	g++ $(WARNING_FLAGS) --std=c++11 -Iinclude -o tester -g $(TESTSUITE_CPP_FILES)