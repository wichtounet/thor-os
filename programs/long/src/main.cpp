//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/print.hpp>

volatile uint64_t current = 45;

uint64_t fibonacci_slow(uint64_t s){
    if(s == 1 || s == 2){
        return current;
    }

    return fibonacci_slow(s - 1) + fibonacci_slow(s - 2);
}

int main(){
    uint64_t i = 0;

    tlib::print_line("START");

    while(i < 10){
        tlib::print_line(fibonacci_slow(current));
        ++i;
    }

    tlib::print_line("END");

    return 0;
}
