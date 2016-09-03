//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <vector.hpp>
#include <algorithms.hpp>

#include "test.hpp"

namespace {

void test_base(){
    std::vector<int> a{1, 0, 0, 2, 3, 4};

    check(a.size() == 6, "Invalid vector:size");
    check(a[0] == 1, "Invalid vector:[]");
    check(a[5] == 4, "Invalid vector:[]");

    check(*a.begin() == 1);
}

void test_erase(){
    std::vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(2);

    check(a.size() == 5, "Invalid vector:size");
    check(a[0] == 1, "Invalid vector:[]");
    check(a[1] == 0, "Invalid vector:[]");
    check(a[2] == 2, "Invalid vector:[]");
    check(a[3] == 3, "Invalid vector:[]");
    check(a[4] == 4, "Invalid vector:[]");

    check(*a.begin() == 1);

    a.erase(a.begin() + 2);

    check(a.size() == 4, "Invalid vector:size");
    check(a[0] == 1, "Invalid vector:[]");
    check(a[1] == 0, "Invalid vector:[]");
    check(a[2] == 3, "Invalid vector:[]");
    check(a[3] == 4, "Invalid vector:[]");
}

void test_erase_range(){
    std::vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(a.begin() + 1, a.begin() + 4);

    check(a.size() == 3, "Invalid vector:size");
    check(a[0] == 1, "Invalid vector:[]");
    check(a[1] == 3, "Invalid vector:[]");
    check(a[2] == 4, "Invalid vector:[]");

    check(*a.begin() == 1);
}

void test_erase_remove(){
    std::vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove(a.begin(), a.end(), 0), a.end());

    check(a.size() == 4, "Invalid vector:size");
    check(a[0] == 1, "Invalid vector:[]");
    check(a[1] == 2, "Invalid vector:[]");
    check(a[2] == 3, "Invalid vector:[]");
    check(a[3] == 4, "Invalid vector:[]");

    check(*a.begin() == 1);
}

void test_erase_remove_if(){
    std::vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove_if(a.begin(), a.end(), [](int value){
        return value == 1 || value == 3;
    }), a.end());

    check(a.size() == 4, "Invalid vector:size");
    check(a[0] == 0, "Invalid vector:[]");
    check(a[1] == 0, "Invalid vector:[]");
    check(a[2] == 2, "Invalid vector:[]");
    check(a[3] == 4, "Invalid vector:[]");

    check(*a.begin() == 0);
}

} //end of anonymous namespace

void vector_tests(){
    test_base();
    test_erase();
    test_erase_range();
    test_erase_remove();
    test_erase_remove_if();
}
