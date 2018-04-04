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

auto message = "I'm one";

int main(){
    while(true){
        fibonacci_slow(current);
        tlib::print_line(message);
    }

    return 0;
}
