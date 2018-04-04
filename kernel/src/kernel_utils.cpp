//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "kernel_utils.hpp"
#include "print.hpp"

void print_stack(const char* s, size_t check){
    printf("%s stack: %u (16B-a:%u) \n", s, check, static_cast<size_t>(check % 16));
}
