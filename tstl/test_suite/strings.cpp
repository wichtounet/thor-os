//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <string.hpp>

void check(bool condition, const char* message){
    if(!condition){
        printf("Check failed: \"%s\"\n", message);
    }
}

void check_equals(long value, long expected, const char* message){
    if(value != expected){
        printf("Check failed: \"%s\"\n", message);
        printf("\t expected: %ld was: %ld\n", expected, value);
    }
}

namespace {

void test_small(){
    std::string s("asdf");

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 4, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "asdf") == 0, "Invalid content");

    s += "1234";

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 8, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "asdf1234") == 0, "Invalid content");
}

void test_empty(){
    std::string s;

    check(s.empty(), "String must be empty");
    check(s.size() == 0, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
}

void test_limit(){
    std::string s("123456789abcdef");

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 15, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdef") == 0, "Invalid content");

    std::string s2("123456789abcdefg");

    check(!s2.empty(), "String mustn't be empty");
    check(s2.size() == 16, "Invalid size");
    check(s2.capacity() == 17, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789abcdefg") == 0, "Invalid content");
}

void test_grow(){
    std::string s("123456789abcdef");

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 15, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdef") == 0, "Invalid content");

    s += 'g';

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 16, "Invalid size");
    check_equals(s.capacity(), 32, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdefg") == 0, "Invalid content");

    s += 'g';

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 17, "Invalid size");
    check_equals(s.capacity(), 32, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdefgg") == 0, "Invalid content");
}

void test_concat(){
    std::string s1("123456789");
    std::string s2("123456789");

    s1 += s2;

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 18, "Invalid size");
    check_equals(s1.capacity(), 32, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789123456789") == 0, "Invalid content");

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    std::string s3;
    s3 = s1 + s2;

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 18, "Invalid size");
    check_equals(s1.capacity(), 32, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789123456789") == 0, "Invalid content");

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    check(!s3.empty(), "String mustn't be empty");
    check_equals(s3.size(), 27, "Invalid size");
    check_equals(s3.capacity(), 32, "Invalid capacity");
    check(strcmp(s3.c_str(), "123456789123456789123456789") == 0, "Invalid content");
}

void test_move(){
    std::string s1("123456789");

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 9, "Invalid size");
    check_equals(s1.capacity(), 16, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789") == 0, "Invalid content");

    std::string s2(std::move(s1));

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    check(s1.empty(), "Moved string must be empty");
    check_equals(s1.size(), 0, "Invalid size");

    std::string s3;
    s3 = std::move(s2);

    check(!s3.empty(), "String mustn't be empty");
    check_equals(s3.size(), 9, "Invalid size");
    check_equals(s3.capacity(), 16, "Invalid capacity");
    check(strcmp(s3.c_str(), "123456789") == 0, "Invalid content");

    check(s2.empty(), "Moved string must be empty");
    check_equals(s2.size(), 0, "Invalid size");

    s3 += std::string("123456789");

    check(!s3.empty(), "String mustn't be empty");
    check_equals(s3.size(), 18, "Invalid size");
    check_equals(s3.capacity(), 32, "Invalid capacity");
    check(strcmp(s3.c_str(), "123456789123456789") == 0, "Invalid content");

    std::string s4("asdf");

    s4 = std::move(s3);

    check(!s4.empty(), "String mustn't be empty");
    check_equals(s4.size(), 18, "Invalid size");
    check_equals(s4.capacity(), 32, "Invalid capacity");
    check(strcmp(s4.c_str(), "123456789123456789") == 0, "Invalid content");

    std::string s5("asdf");
    s4 = std::move(s5);

    check(!s4.empty(), "String mustn't be empty");
    check_equals(s4.size(), 4, "Invalid size");
    check_equals(s4.capacity(), 32, "Invalid capacity");
    check(strcmp(s4.c_str(), "asdf") == 0, "Invalid content");
}

void test_large(){
    char large_buffer[4097];

    char buffer[1025];
    for(int i = 0; i < 1024; ++i){
        buffer[i] = 'a' + (i % 9);
    }
    buffer[1024] = '\0';

    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 1024; ++j){
            large_buffer[i * 1024 + j] = buffer[j];
        }
    }
    large_buffer[4096] = '\0';

    std::string s(buffer);

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 1024, "Invalid size");
    check_equals(s.capacity(), 1025, "Invalid capacity");
    check(strcmp(s.c_str(), buffer) == 0, "Invalid content");

    s += buffer;
    s += buffer;
    s += buffer;

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 4096, "Invalid size");
    check_equals(s.capacity(), 4100, "Invalid capacity");
    check(strcmp(s.c_str(), large_buffer) == 0, "Invalid content");
}

} //end of anonymous namespace

void string_tests(){
    test_small();
    test_empty();
    test_limit();
    test_grow();
    test_concat();
    test_move();
    test_large();
}
