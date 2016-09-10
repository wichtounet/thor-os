//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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

void test_push_front(){
    std::vector<int> a{1, 0, 0, 2, 3, 4};

    a.push_front(99);

    check(a.size() == 7, "Invalid vector:size");
    check(a[0] == 99, "Invalid vector:[]");
    check(a[1] == 1, "Invalid vector:[]");
    check(a[2] == 0, "Invalid vector:[]");
    check(a[3] == 0, "Invalid vector:[]");
    check(a[4] == 2, "Invalid vector:[]");
    check(a[5] == 3, "Invalid vector:[]");
    check(a[6] == 4, "Invalid vector:[]");

    check(*a.begin() == 99);
}

void test_reverse_iterator(){
    std::vector<int> a{1, 0, 0, 2, 3, 4};

    auto it = a.rbegin();
    auto end = a.rend();

    check(it != end, "Invalid reverse iterator");
    check(*it == 4, "Invalid reverse iterator");

    ++it;
    check(it != end, "Invalid reverse iterator");
    check(*it == 3, "Invalid reverse iterator");

    ++it;
    check(it != end, "Invalid reverse iterator");
    check(*it == 2, "Invalid reverse iterator");

    ++it;
    check(it != end, "Invalid reverse iterator");
    check(*it == 0, "Invalid reverse iterator");

    ++it;
    check(it != end, "Invalid reverse iterator");
    check(*it == 0, "Invalid reverse iterator");

    ++it;
    check(it != end, "Invalid reverse iterator");
    check(*it == 1, "Invalid reverse iterator");

    ++it;
    check(it == end, "Invalid reverse iterator");
}

} //end of anonymous namespace

void vector_tests(){
    test_base();
    test_erase();
    test_erase_range();
    test_erase_remove();
    test_erase_remove_if();
    test_push_front();
    test_reverse_iterator();
}
