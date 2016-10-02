//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#include "test.hpp"

void string_tests();
void function_tests();
void tuple_tests();
void vector_tests();
void deque_tests();
void expected_tests();
void list_tests();
void traits_tests();
void algorithms_tests();
void circular_buffer_tests();
void shared_ptr_tests();

int main(){
    string_tests();
    traits_tests();
    algorithms_tests();
    circular_buffer_tests();
    tuple_tests();
    vector_tests();
    deque_tests();
    expected_tests();
    shared_ptr_tests();
    list_tests();
    function_tests();

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
