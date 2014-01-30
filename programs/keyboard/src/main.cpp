//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>

const char* source = "Hello world";

int main(){
    for(int i = 0; i < 10; ++i){
        auto c = read_char();
        print(c);
    }

    exit(0);
}