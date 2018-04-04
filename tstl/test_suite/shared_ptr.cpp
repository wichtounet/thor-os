//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <shared_ptr.hpp>

#include "test.hpp"

namespace {

void test_base(){
    std::shared_ptr<int> a(new int);

    *a.get() = 99;

    check(*a.get() == 99);
}

struct foo {
    int bar = 0;

    int test(){
        return bar;
    }

    void set(int bar){
        this->bar = bar;
    }
};

void test_struct(){
    std::shared_ptr<foo> a(new foo);

    check(a->test() == 0);

    a->set(666);

    check(a->test() == 666);
}

struct kiss {
    int* ref;
    kiss(int* ref) : ref(ref) {}
    ~kiss(){
        ++(*ref);
    }
};

void test_destructor() {
    int counter = 0;

    {
        std::shared_ptr<kiss> a(new kiss(&counter));
        auto b = a;
        auto c = a;
    }

    check(counter == 1, "destruct: Invalid destructors");
}

void test_make_shared() {
    int counter = 0;

    {
        auto a = std::make_shared<kiss>(&counter);
        auto b = a;
        auto c = a;
    }

    check(counter == 1, "make_shared: Invalid destructors");
}

} //end of anonymous namespace

void shared_ptr_tests(){
    test_base();
    test_struct();
    test_destructor();
    test_make_shared();
}
