//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>

int main(int argc, char* argv[]){
    for(size_t i = 0; i < size_t(argc); ++i){
        print_line(argv[i]);
    }
    exit(0);
}
