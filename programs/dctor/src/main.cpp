//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
