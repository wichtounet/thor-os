//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VIRTUAL_DEBUG_H
#define VIRTUAL_DEBUG_H

//TODO Integrate Bochs Parallel debugging
//TODO Integrate Qemu Serial debugging

#include "kernel_utils.hpp"

#define BOCHS_E9 0xE9

inline bool is_bochs_e9(){
    auto e9 = in_byte(0xe9);
    return e9 == BOCHS_E9;
}

inline void bochs_print_char(char c){
    out_byte(BOCHS_E9, c);
}

inline void bochs_print(const char* s){
    for(uint64_t i = 0; s[i] != '\0'; ++i){
        bochs_print_char(s[i]);
    }
}

inline void virtual_debug(const char* s){
    if(is_bochs_e9()){
        bochs_print(s);
    }
}

#endif
