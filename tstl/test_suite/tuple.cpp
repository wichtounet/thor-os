//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <tuple.hpp>

#include "test.hpp"

namespace {

void test_base(){
    std::tuple<int, double, char> t;

    std::get<0>(t) = 99;
    std::get<1>(t) = 1.01;
    std::get<2>(t) = 'z';

    check(std::get<0>(t) == 99, "Invalid std::get");
    check(std::get<1>(t) == 1.01, "Invalid std::get");
    check(std::get<2>(t) == 'z', "Invalid std::get");
}

void test_make_tuple(){
    auto t = std::make_tuple(99, 1.01, 'z');

    check(std::get<0>(t) == 99, "Invalid std::get");
    check(std::get<1>(t) == 1.01, "Invalid std::get");
    check(std::get<2>(t) == 'z', "Invalid std::get");
}

void test_tie(){
    auto t = std::make_tuple(99, 1.01, 'z');

    int a;
    double b;
    char c;
    std::tie(a, b, c) = t;

    check(a == 99, "Invalid std::get");
    check(b == 1.01, "Invalid std::get");
    check(c == 'z', "Invalid std::get");
}

void test_more(){
    auto t  = std::tuple<int, double, int>();
    auto t2 = std::tuple<int, double, int>(1, 2, 3);
    auto t3(t2);

    auto t4 = std::tuple<int, double, int>(1, 2, 2);
    auto t5 = std::tuple<int, double, int>(1, 2, 4);

    check(std::tuple_size<decltype(t)>::value == 3, "Invalid tuple_size");
    check(std::tuple_size<decltype(t2)>::value == 3, "Invalid tuple_size");
    check(std::tuple_size<decltype(t3)>::value == 3, "Invalid tuple_size");

    check(std::get<0>(t2) == 1, "Invalid std::get<N>");
    check(std::get<1>(t2) == 2, "Invalid std::get<N>");
    check(std::get<2>(t2) == 3, "Invalid std::get<N>");

    check(std::get<0>(t3) == 1, "Invalid std::get<N>");
    check(std::get<1>(t3) == 2, "Invalid std::get<N>");
    check(std::get<2>(t3) == 3, "Invalid std::get<N>");

    check(std::get<0>(t4) == 1, "Invalid std::get<N>");
    check(std::get<1>(t4) == 2, "Invalid std::get<N>");
    check(std::get<2>(t4) == 2, "Invalid std::get<N>");

    check(std::get<0>(t5) == 1, "Invalid std::get<N>");
    check(std::get<1>(t5) == 2, "Invalid std::get<N>");
    check(std::get<2>(t5) == 4, "Invalid std::get<N>");

    t5 = t4;

    check(std::get<0>(t5) == 1, "Invalid std::get<N>");
    check(std::get<1>(t5) == 2, "Invalid std::get<N>");
    check(std::get<2>(t5) == 2, "Invalid std::get<N>");
}

} //end of anonymous namespace

void tuple_tests(){
    test_base();
    test_make_tuple();
    test_tie();
    test_more();
}
