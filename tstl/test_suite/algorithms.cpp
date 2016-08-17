//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <type_traits.hpp>
#include <string.hpp>

#include "test.hpp"

namespace {

void test_copy(){
    char a[25];
    const char* b = "Hello,Thor.OS.OS";

    a[0] = 'a';
    a[3] = 'b';
    a[20] = 'd';
    a[24] = 'e';

    std::copy(b, b + 16, a + 4);

    check(a[0] == 'a', "Invalid copy");
    check(a[3] == 'b', "Invalid copy");
    for(size_t i = 4; i < 20; ++i){
        check(a[i] == b[i - 4], "Invalid copy");
    }
    check(a[20] == 'd', "Invalid copy");
    check(a[24] == 'e', "Invalid copy");
}

void test_copy_n(){
    char a[25];
    const char* b = "hELLO,tHOR.os.os";

    a[0] = 'a';
    a[3] = 'b';
    a[20] = 'd';
    a[24] = 'e';

    std::copy_n(b, 16, a + 4);

    check(a[0] == 'a', "Invalid copy_n");
    check(a[3] == 'b', "Invalid copy_n");
    for(size_t i = 4; i < 20; ++i){
        check(a[i] == b[i - 4], "Invalid copy_n");
    }
    check(a[20] == 'd', "Invalid copy_n");
    check(a[24] == 'e', "Invalid copy_n");
}

void test_fill_n(){
    char test[25];

    test[0] = 'a';
    test[3] = 'b';
    test[20] = 'd';
    test[24] = 'e';

    std::fill_n(test + 4, 16, 'Z');

    check(test[0] == 'a', "Invalid fill_n");
    check(test[3] == 'b', "Invalid fill_n");
    for(size_t i = 4; i < 20; ++i){
        check(test[i] == 'Z', "Invalid fill_n");
    }
    check(test[20] == 'd', "Invalid fill_n");
    check(test[24] == 'e', "Invalid fill_n");
}

void test_fill(){
    char test[25];

    test[0] = 'a';
    test[3] = 'b';
    test[20] = 'd';
    test[24] = 'e';

    std::fill(test + 4, test + 20, 'Z');

    check(test[0] == 'a', "Invalid fill_n");
    check(test[3] == 'b', "Invalid fill_n");
    for(size_t i = 4; i < 20; ++i){
        check(test[i] == 'Z', "Invalid fill_n");
    }
    check(test[20] == 'd', "Invalid fill_n");
    check(test[24] == 'e', "Invalid fill_n");
}

void test_clear_n(){
    char test[25];

    test[0] = 'a';
    test[3] = 'b';
    test[20] = 'd';
    test[24] = 'e';

    std::fill_n(test + 4, 16, 0);

    check(test[0] == 'a', "Invalid fill_n");
    check(test[3] == 'b', "Invalid fill_n");
    for(size_t i = 4; i < 20; ++i){
        check(test[i] == 0, "Invalid fill_n");
    }
    check(test[20] == 'd', "Invalid fill_n");
    check(test[24] == 'e', "Invalid fill_n");
}

void test_clear(){
    char test[25];

    test[0] = 'a';
    test[3] = 'b';
    test[20] = 'd';
    test[24] = 'e';

    std::fill(test + 4, test + 20, 0);

    check(test[0] == 'a', "Invalid fill_n");
    check(test[3] == 'b', "Invalid fill_n");
    for(size_t i = 4; i < 20; ++i){
        check(test[i] == 0, "Invalid fill_n");
    }
    check(test[20] == 'd', "Invalid fill_n");
    check(test[24] == 'e', "Invalid fill_n");
}

} //end of anonymous namespace

void algorithms_tests(){
    test_copy();
    test_copy_n();
    test_fill();
    test_fill_n();
    test_clear();
    test_clear_n();
}
