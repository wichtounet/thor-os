//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>
#include <new>

#include <function.hpp>

#include "test.hpp"

namespace {

int foo(int& ref){
    ++ref;
    return ref;
}

void test_function_ptr(){
    std::function<int(int&)> f(foo);

    int a = 4;
    auto c = f(a);

    check(c == 5, "function: function_ptr error");
    check(a == 5, "function: function_ptr error");
}

void test_lambda(){
    auto l = [](int& ref){ ++ref; return ref; };
    std::function<int(int&)> f(l);

    int a = 2;
    auto c = f(a);

    check(c == 3, "function: lambda error");
    check(a == 3, "function: lambda error");
}

void test_lambda_state(){
    int a = 3;

    auto l = [&a](){ ++a; return a; };
    std::function<int()> f(l);

    auto c = f();

    check(c == 4, "function: lambda error");
    check(a == 4, "function: lambda error");
}

} //end of anonymous namespace

void function_tests(){
    test_function_ptr();
    test_lambda();
    test_lambda_state();
}
