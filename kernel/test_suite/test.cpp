//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#include "test.hpp"

void path_tests();

int main(){
    path_tests();

    printf("All tests finished\n");

    return 0;
}

// TODO Avoid that duplication

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

void check(bool condition, const char* where, size_t line){
    if(!condition){
        printf("%s:%lu Check failed\n", where, line);
    }
}

void check(bool condition, const char* message, const char* where, size_t line){
    if(!condition){
        printf("%s:%lu Check failed: \"%s\"\n", where, line, message);
    }
}

void check_equals(long value, long expected, const char* message, const char* where, size_t line){
    if(value != expected){
        printf("%s:%lu Check failed: \"%s\"\n", where, line, message);
        printf("\t expected: %ld was: %ld\n", expected, value);
    }
}

void check_equals(long value, long expected, const char* where, size_t line){
    if(value != expected){
        printf("%s:%lu Check failed", where, line);
        printf("\t expected: %ld was: %ld\n", expected, value);
    }
}
