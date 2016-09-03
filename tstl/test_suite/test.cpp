//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cstdio>
#include <cstring>

#include "test.hpp"

void string_tests();
void tuple_tests();
void vector_tests();
void traits_tests();
void algorithms_tests();
void circular_buffer_tests();

int main(){
    string_tests();
    traits_tests();
    algorithms_tests();
    circular_buffer_tests();
    tuple_tests();
    vector_tests();

    printf("All tests finished\n");

    return 0;
}

void check(bool condition){
    if(!condition){
        printf("Check failed\n");
    }
}

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
