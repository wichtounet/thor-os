//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <small_vector.hpp>
#include <algorithms.hpp>

#include "test.hpp"

namespace {

void test_small_0(){
    std::small_vector<int8_t> a;

    CHECK_EQUALS(a.size(), 0, "Invalid small_vector:size()");
    CHECK_EQUALS(a.capacity(), 16, "Invalid small_vector:capacity()");
}

void test_small_1(){
    std::small_vector<int8_t> a(3);

    CHECK_EQUALS(a.size(), 3, "Invalid small_vector:size()");
    CHECK_EQUALS(a.capacity(), 16, "Invalid small_vector:capacity()");
}

void test_small_2(){
    std::small_vector<int8_t> a{1, 0, 0, 2, 3, 4};

    CHECK_EQUALS(a.size(), 6, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[5], 4, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 4, "Invalid small_vector::end()");
}

void test_base(){
    std::small_vector<int32_t> a{1, 0, 0, 2, 3, 4};

    CHECK_EQUALS(a.size(), 6, "Invalid small_vector:size");
    CHECK_EQUALS(a.capacity(), 6, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[5], 4, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 4, "Invalid small_vector::end()");
}

void test_push_back_0(){
    std::small_vector<int8_t> a{1, 0, 0, 2, 3, 4};

    a.push_back(5);

    CHECK_EQUALS(a.size(), 7, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[6], 5, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 5, "Invalid small_vector::end()");

    a.push_back(15);

    CHECK_EQUALS(a.size(), 8, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[7], 15, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 15, "Invalid small_vector::end()");
}

void test_push_back_1(){
    std::small_vector<int32_t> a{1, 0, 0, 2, 3, 4};

    a.push_back(5);

    CHECK_EQUALS(a.size(), 7, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[6], 5, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 5, "Invalid small_vector::end()");

    a.push_back(15);

    CHECK_EQUALS(a.size(), 8, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[7], 15, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 15, "Invalid small_vector::end()");
}

void test_push_back_2(){
    std::small_vector<int32_t> a{1, 0, 0};

    a.push_back(5);

    CHECK_EQUALS(a.size(), 4, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[3], 5, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 5, "Invalid small_vector::end()");

    a.push_back(15);

    CHECK_EQUALS(a.size(), 5, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[4], 15, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 15, "Invalid small_vector::end()");

    a.push_back(33);

    CHECK_EQUALS(a.size(), 6, "Invalid small_vector:size");
    CHECK_EQUALS(a[0], 1, "Invalid small_vector:[]");
    CHECK_EQUALS(a[5], 33, "Invalid small_vector:[]");
    CHECK_EQUALS(*a.begin(), 1, "Invalid small_vector::begin()");
    CHECK_EQUALS(*(a.end() - 1), 33, "Invalid small_vector::end()");
}

void test_erase(){
    std::small_vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(2);

    check(a.size() == 5, "Invalid small_vector:size");
    check(a[0] == 1, "Invalid small_vector:[]");
    check(a[1] == 0, "Invalid small_vector:[]");
    check(a[2] == 2, "Invalid small_vector:[]");
    check(a[3] == 3, "Invalid small_vector:[]");
    check(a[4] == 4, "Invalid small_vector:[]");

    check(*a.begin() == 1);

    a.erase(a.begin() + 2);

    check(a.size() == 4, "Invalid small_vector:size");
    check(a[0] == 1, "Invalid small_vector:[]");
    check(a[1] == 0, "Invalid small_vector:[]");
    check(a[2] == 3, "Invalid small_vector:[]");
    check(a[3] == 4, "Invalid small_vector:[]");
}

void test_erase_range(){
    std::small_vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(a.begin() + 1, a.begin() + 4);

    check(a.size() == 3, "Invalid small_vector:size");
    check(a[0] == 1, "Invalid small_vector:[]");
    check(a[1] == 3, "Invalid small_vector:[]");
    check(a[2] == 4, "Invalid small_vector:[]");

    check(*a.begin() == 1);
}

void test_erase_remove(){
    std::small_vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove(a.begin(), a.end(), 0), a.end());

    check(a.size() == 4, "Invalid small_vector:size");
    check(a[0] == 1, "Invalid small_vector:[]");
    check(a[1] == 2, "Invalid small_vector:[]");
    check(a[2] == 3, "Invalid small_vector:[]");
    check(a[3] == 4, "Invalid small_vector:[]");

    check(*a.begin() == 1);
}

void test_erase_remove_if(){
    std::small_vector<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove_if(a.begin(), a.end(), [](int value){
        return value == 1 || value == 3;
    }), a.end());

    check(a.size() == 4, "Invalid small_vector:size");
    check(a[0] == 0, "Invalid small_vector:[]");
    check(a[1] == 0, "Invalid small_vector:[]");
    check(a[2] == 2, "Invalid small_vector:[]");
    check(a[3] == 4, "Invalid small_vector:[]");

    check(*a.begin() == 0);
}

void test_push_front(){
    std::small_vector<int> a{1, 0, 0, 2, 3, 4};

    a.push_front(99);

    check(a.size() == 7, "Invalid small_vector:size");
    check(a[0] == 99, "Invalid small_vector:[]");
    check(a[1] == 1, "Invalid small_vector:[]");
    check(a[2] == 0, "Invalid small_vector:[]");
    check(a[3] == 0, "Invalid small_vector:[]");
    check(a[4] == 2, "Invalid small_vector:[]");
    check(a[5] == 3, "Invalid small_vector:[]");
    check(a[6] == 4, "Invalid small_vector:[]");

    check(*a.begin() == 99);
}

void test_reverse_iterator(){
    std::small_vector<int> a{1, 0, 0, 2, 3, 4};

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
    kiss() {} // for small_vector
    kiss(int* ref) : ref(ref) {}
    ~kiss(){
        ++(*ref);
    }
};

void test_destructor() {
    int counter = 0;

    {
        std::small_vector<kiss> vec;
        vec.reserve(3);
        vec.emplace_back(&counter);
        vec.emplace_back(&counter);
        vec.emplace_back(&counter);
    }

    CHECK_EQUALS(counter, 3, "destruct: Invalid destructors");
}

void test_clear() {
    int counter = 0;

    std::small_vector<kiss> vec;
    vec.reserve(3);
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.clear();

    check(counter == 3, "clear: Invalid destructors");
}

void test_pop_back() {
    int counter = 0;

    std::small_vector<kiss> vec;
    vec.reserve(3);
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.emplace_back(&counter);
    vec.pop_back();

    check(counter == 1, "pop_back: Invalid destructors");
}

void test_lots() {
    std::small_vector<size_t> vec;

    for(size_t i = 0; i < 1000; ++i){
        vec.push_back(i);
    }

    for(size_t i = 10000; i < 11000; ++i){
        vec.push_front(i);
    }

    for(size_t i = 0; i < 1000; ++i){
        CHECK_EQUALS(vec[i], 10999 - i, "test_lots: invalid []");
    }

    for(size_t i = 1000; i < 2000; ++i){
        CHECK_EQUALS(vec[i], i - 1000, "test_lots: invalid []");
    }

    CHECK_EQUALS(vec.size(), 2000, "test_lots: invalid size()");
    CHECK_EQUALS(vec.front(), 10999, "test_lots: invalid front()");
    CHECK_EQUALS(vec.back(), 999, "test_lots: invalid back()");

    for(size_t i = 0; i < 1000; ++i){
        vec.pop_back();
    }

    CHECK_EQUALS(vec.size(), 1000, "test_lots: invalid size()");
    CHECK_EQUALS(vec.front(), 10999, "test_lots: invalid front()");
    CHECK_EQUALS(vec.back(), 10000, "test_lots: invalid back()");

    for(size_t i = 0; i < 500; ++i){
        vec.erase(vec.begin());
    }

    CHECK_EQUALS(vec.size(), 500, "test_lots: invalid size()");
    CHECK_EQUALS(vec.front(), 10499, "test_lots: invalid front()");
    CHECK_EQUALS(vec.back(), 10000, "test_lots: invalid back()");
}

} //end of anonymous namespace

void small_vector_tests(){
    test_small_0();
    test_small_1();
    test_small_2();
    test_base();
    test_push_back_0();
    test_push_back_1();
    test_push_back_2();
    test_erase();
    test_erase_range();
    test_erase_remove();
    test_erase_remove_if();
    test_push_front();
    test_reverse_iterator();
    test_destructor();
    test_clear();
    test_pop_back();
    test_lots();
}
