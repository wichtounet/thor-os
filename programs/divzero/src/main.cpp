//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <system.hpp>

volatile int a = 0;

int main(int /*argc*/, char* /*argv*/[]){
    a = 42 / a;

    exit(a);
}
