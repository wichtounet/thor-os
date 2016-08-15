//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <system.hpp>

int main(int argc, char* argv[]);

extern "C" {

void _start(int argc, char* argv[]) __attribute__((section(".start")));

void _start(int argc, char* argv[]){
    auto code = main(argc, argv);
    exit(code);
}

} // end of extern C
