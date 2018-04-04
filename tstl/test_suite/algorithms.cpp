//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>
#include <cstdlib>

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

void test_fill_n_2(){
    char* test[4];

    // This must compile
    std::fill_n(test, 4, nullptr);

    check(test[0] == nullptr, "Invalid fill_n");
    check(test[1] == nullptr, "Invalid fill_n");
    check(test[2] == nullptr, "Invalid fill_n");
    check(test[3] == nullptr, "Invalid fill_n");
}

struct POD {
    int a;
};

void test_fill_n_3(){
    POD test[4];

    POD a{99};

    // This must compile
    std::fill_n(test, 4, a);

    check(test[0].a == 99, "Invalid fill_n");
    check(test[1].a == 99, "Invalid fill_n");
    check(test[2].a == 99, "Invalid fill_n");
    check(test[3].a == 99, "Invalid fill_n");
}

} //end of anonymous namespace

void algorithms_tests(){
    test_copy();
    test_copy_n();
    test_fill();
    test_fill_n();
    test_fill_n_2();
    test_fill_n_3();
    test_clear();
    test_clear_n();
}
