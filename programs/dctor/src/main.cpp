//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <tlib/system.hpp>
#include <tlib/print.hpp>

namespace {

struct A {
    A(){
        tlib::print_line("dctor: Constructor called");
    }

    ~A(){
        tlib::print_line("dctor: Destructor called");
    }
};

A a;

} // end of anonymous namespace

int main(int, char*[]){
    tlib::print_line("dctor: main function called");

    return 0;
}
