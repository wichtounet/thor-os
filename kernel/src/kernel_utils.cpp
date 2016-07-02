//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "kernel_utils.hpp"
#include "console.hpp"

void print_stack(const char* s, size_t check){
    printf("%s stack: %u (16B-a:%u) \n", s, check, static_cast<size_t>(check % 16));
}
