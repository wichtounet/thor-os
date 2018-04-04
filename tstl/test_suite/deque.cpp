//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <deque.hpp>

#include "test.hpp"

namespace {

void test_base(){
    std::deque<int> a{1, 0, 0, 2, 3, 4};

    check_equals(a.size(), 6, "test_base: Invalid deque:size");
    check_equals(a[0], 1, "test_base: Invalid deque:[]");
    check_equals(a[1], 0, "test_base: Invalid deque:[]");
    check_equals(a[2], 0, "test_base: Invalid deque:[]");
    check_equals(a[3], 2, "test_base: Invalid deque:[]");
    check_equals(a[4], 3, "test_base: Invalid deque:[]");
    check_equals(a[5], 4, "test_base: Invalid deque:[]");

    check(*a.begin() == 1, "test_base: invalid begin()");
    check(a.front() == 1, "test_base: invalid front()");
    check(a.back() == 4, "test_base: invalid back()");
}

void test_erase(){
    std::deque<int> a{1, 0, 0, 2, 3, 4};

    a.erase(2);

    check(a.size() == 5, "Invalid deque:size");
    check(a[0] == 1, "Invalid deque:[]");
    check(a[1] == 0, "Invalid deque:[]");
    check(a[2] == 2, "Invalid deque:[]");
    check(a[3] == 3, "Invalid deque:[]");
    check(a[4] == 4, "Invalid deque:[]");

    check(*a.begin() == 1);

    a.erase(a.begin() + 2);

    check(a.size() == 4, "Invalid deque:size");
    check(a[0] == 1, "Invalid deque:[]");
    check(a[1] == 0, "Invalid deque:[]");
    check(a[2] == 3, "Invalid deque:[]");
    check(a[3] == 4, "Invalid deque:[]");
}

void test_erase_range(){
    std::deque<int> a{1, 0, 0, 2, 3, 4};

    a.erase(a.begin() + 1, a.begin() + 4);

    check_equals(a.size(), 3, "Invalid deque:size");
    check_equals(a[0], 1, "Invalid deque:[]");
    check_equals(a[1], 3, "Invalid deque:[]");
    check_equals(a[2], 4, "Invalid deque:[]");

    check(*a.begin() == 1);
}

void test_erase_remove(){
    std::deque<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove(a.begin(), a.end(), 0), a.end());

    check(a.size() == 4, "Invalid deque:size");
    check(a[0] == 1, "Invalid deque:[]");
    check(a[1] == 2, "Invalid deque:[]");
    check(a[2] == 3, "Invalid deque:[]");
    check(a[3] == 4, "Invalid deque:[]");

    check(*a.begin() == 1);
}

void test_erase_remove_if(){
    std::deque<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove_if(a.begin(), a.end(), [](int value){
        return value == 1 || value == 3;
    }), a.end());

    check(a.size() == 4, "Invalid deque:size");
    check(a[0] == 0, "Invalid deque:[]");
    check(a[1] == 0, "Invalid deque:[]");
    check(a[2] == 2, "Invalid deque:[]");
    check(a[3] == 4, "Invalid deque:[]");

    check(*a.begin() == 0);
}

void test_mix(){
    std::deque<int> a;

    a.push_front(99);
    a.push_back(33);
    a.push_front(11);
    a.push_back(22);

    check(a.size() == 4, "Invalid deque:size");
    check(a[0] == 11, "Invalid deque:[]");
    check(a[1] == 99, "Invalid deque:[]");
    check(a[2] == 33, "Invalid deque:[]");
    check(a[3] == 22, "Invalid deque:[]");

    check(*a.begin() == 11, "test_mix: invalid begin()");
    check(a.front() == 11, "test_mix: invalid front()");
    check(a.back() == 22, "test_mix: invalid back()");
}

void test_push_front(){
    std::deque<int> a{1, 0, 0, 2, 3, 4};

    a.push_front(99);

    check(a.size() == 7, "Invalid deque:size");
    check(a[0] == 99, "Invalid deque:[]");
    check(a[1] == 1, "Invalid deque:[]");
    check(a[2] == 0, "Invalid deque:[]");
    check(a[3] == 0, "Invalid deque:[]");
    check(a[4] == 2, "Invalid deque:[]");
    check(a[5] == 3, "Invalid deque:[]");
    check(a[6] == 4, "Invalid deque:[]");

    check_equals(*a.begin(), 99, "test_push_front: invalid begin()");
    check_equals(a.front(), 99, "test_push_front: invalid front()");
    check_equals(a.back(), 4, "test_push_front: invalid back()");
}

void test_reverse_iterator(){
    std::deque<int> a{1, 0, 0, 2, 3, 4};

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

struct kiss {
    int* ref;
    kiss() {} // for deque
    kiss(int* ref) : ref(ref) {}
    ~kiss(){
        ++(*ref);
    }
};

void test_destructor() {
    int counter = 0;

    {
        std::deque<kiss> vec;
        vec.emplace_back(&counter);
        vec.emplace_back(&counter);
        vec.emplace_back(&counter);
    }

    check_equals(counter, 3, "destruct: Invalid destructors");
}

void test_clear() {
    int counter = 0;

    std::deque<kiss> vec;
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.clear();

    check_equals(counter, 3, "clear: Invalid destructors");
}

void test_pop_back_dst() {
    int counter = 0;

    std::deque<kiss> vec;
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.pop_back();

    check_equals(counter, 1, "pop_back_dst: Invalid destructors");
}

void test_lots() {
    std::deque<size_t> vec;

    for(size_t i = 0; i < 1000; ++i){
        vec.push_back(i);
    }

    for(size_t i = 10000; i < 11000; ++i){
        vec.push_front(i);
    }

    for(size_t i = 0; i < 1000; ++i){
        check_equals(vec[i], 10999 - i, "test_lots: invalid []");
    }

    for(size_t i = 1000; i < 2000; ++i){
        check_equals(vec[i], i - 1000, "test_lots: invalid []");
    }

    check_equals(vec.size(), 2000, "test_lots: invalid size()");
    check_equals(vec.front(), 10999, "test_lots: invalid front()");
    check_equals(vec.back(), 999, "test_lots: invalid back()");

    for(size_t i = 0; i < 1000; ++i){
        vec.pop_back();
    }

    check_equals(vec.size(), 1000, "test_lots: invalid size()");
    check_equals(vec.front(), 10999, "test_lots: invalid front()");
    check_equals(vec.back(), 10000, "test_lots: invalid back()");

    for(size_t i = 0; i < 500; ++i){
        vec.pop_front();
    }

    check_equals(vec.size(), 500, "test_lots: invalid size()");
    check_equals(vec.front(), 10499, "test_lots: invalid front()");
    check_equals(vec.back(), 10000, "test_lots: invalid back()");
}

} //end of anonymous namespace

void deque_tests(){
    test_base();
    test_erase();
    test_erase_range();
    test_erase_remove();
    test_erase_remove_if();
    test_mix();
    test_push_front();
    test_reverse_iterator();
    test_destructor();
    test_clear();
    test_pop_back_dst();
    test_lots();
}
