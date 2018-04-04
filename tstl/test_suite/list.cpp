//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <list.hpp>
#include <algorithms.hpp>

#include "test.hpp"

namespace {

void test_base(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    check(a.size() == 6, "Invalid list:size");
    check(a.front()== 1, "Invalid list:[]");
    check(a.back() == 4, "Invalid list:[]");

    check(*a.begin() == 1);
}

template<typename Iterator>
Iterator advance(Iterator it, size_t n){
    for(size_t i = 0; i < n; ++i){
        ++it;
    }

    return it;
}

void test_erase(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    check(!a.empty(), "erase: Invalid list::empty()");

    a.erase(advance(a.begin(), 2));

    check(!a.empty(), "erase: Invalid list::empty()");
    check(a.size() == 5, "erase: Invalid list:size");
    check(a.front() == 1, "erase: Invalid list:front()");
    check(a.back() == 4, "erase: Invalid list:back()");

    check(*a.begin() == 1, "erase: Invalid element 0");
    check(*advance(a.begin(),1) == 0, "erase: Invalid element 1");
    check(*advance(a.begin(),2) == 2, "erase: Invalid element 2");
    check(*advance(a.begin(),3) == 3, "erase: Invalid element 3");
    check(*advance(a.begin(),4) == 4, "erase: Invalid element 4");

    a.erase(a.begin());

    check(a.size() == 4, "erase: Invalid list:size");
    check(a.front() == 0, "erase: Invalid list:front()");
    check(a.back() == 4, "erase: Invalid list:back()");

    check(*a.begin() == 0, "erase: Invalid element 0 ");
    check(*advance(a.begin(),1) == 2, "erase: Invalid element 1");
    check(*advance(a.begin(),2) == 3, "erase: Invalid element 2");
    check(*advance(a.begin(),3) == 4, "erase: Invalid element 3");
}

void test_erase_range(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    a.erase(advance(a.begin(), 1), advance(a.begin(), 4));

    check(a.size() == 3, "erase_range: Invalid list:size");
    check(a.front() == 1, "erase_range: Invalid list:[]");
    check(a.back() == 4, "erase_range: Invalid list:[]");

    check(*a.begin() == 1);
}

void test_erase_remove(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove(a.begin(), a.end(), 0), a.end());

    check(a.size() == 4, "erase_remove: Invalid list:size");
    check(a.front() == 1, "erase_remove: Invalid list:front()");
    check(a.back() == 4, "erase_remove: Invalid list:back()");

    check(*a.begin() == 1, "erase_remove: Invalid begin()");
}

void test_erase_remove_if(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    a.erase(std::remove_if(a.begin(), a.end(), [](int value){
        return value == 1 || value == 3;
    }), a.end());

    check(a.size() == 4, "erase_remove_if: Invalid list:size");
    check(a.front() == 0, "erase_remove_if: Invalid list:front()");
    check(a.back() == 4, "erase_remove_if: Invalid list:back()");

    check(*a.begin() == 0);
}

void test_push_front(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    a.push_front(99);

    check(a.size() == 7, "Invalid list:size");
    check(a.front() == 99, "Invalid list:[]");
    check(a.back() == 4, "Invalid list:[]");

    check(*a.begin() == 99);
}

void test_push_back(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    a.push_back(99);

    check(a.size() == 7, "Invalid list:size");
    check(a.front() == 1, "Invalid list:[]");
    check(a.back() == 99, "Invalid list:[]");

    check(*a.begin() == 1);
}

void test_iterator(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    auto it = a.begin();
    auto end = a.end();

    check(it != end, "Invalid iterator");
    check(*it == 1, "Invalid iterator");

    ++it;
    check(it != end, "Invalid iterator");
    check(*it == 0, "Invalid iterator");

    ++it;
    check(it != end, "Invalid iterator");
    check(*it == 0, "Invalid iterator");

    ++it;
    check(it != end, "Invalid iterator");
    check(*it == 2, "Invalid iterator");

    ++it;
    check(it != end, "Invalid iterator");
    check(*it == 3, "Invalid iterator");

    ++it;
    check(it != end, "Invalid iterator");
    check(*it == 4, "Invalid iterator");

    ++it;
    check(it == end, "Invalid iterator");
}

void test_reverse_iterator(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

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

void test_lots() {
    std::list<size_t> vec;

    for(size_t i = 0; i < 1000; ++i){
        vec.push_back(i);
    }

    for(size_t i = 10000; i < 11000; ++i){
        vec.push_front(i);
    }

    auto it = vec.begin();
    for(size_t i = 0; i < 1000; ++i){
        check_equals(*it, 10999 - i, "test_lots: invalid iterator");
        ++it;
    }

    for(size_t i = 1000; i < 2000; ++i){
        check_equals(*it, i - 1000, "test_lots: invalid iterator");
        ++it;
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

void test_clear(){
    std::list<int> a{1, 0, 0, 2, 3, 4};

    check(!a.empty(), "invalid list::empty()");

    a.push_back(3);
    check(!a.empty(), "invalid list::empty()");

    a.push_front(4);
    check(!a.empty(), "invalid list::empty()");

    a.clear();

    check(a.empty(), "invalid list::empty()");
}

} //end of anonymous namespace

void list_tests(){
    test_base();
    test_erase();
    test_erase_range();
    test_erase_remove();
    test_erase_remove_if();
    test_push_front();
    test_push_back();
    test_iterator();
    test_reverse_iterator();
    test_lots();
    test_clear();
}
