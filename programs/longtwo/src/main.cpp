//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <tlib/print.hpp>

volatile uint64_t current = 45;

uint64_t fibonacci_slow(uint64_t s){
    if(s == 1 || s == 2){
        return current;
    }

    return fibonacci_slow(s - 1) + fibonacci_slow(s - 2);
}

auto message = "I'm two";

int main(){
    while(true){
        fibonacci_slow(current);
        tlib::print_line(message);
    }

    return 0;
}
