//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VIRTUAL_DEBUG_H
#define VIRTUAL_DEBUG_H

//TODO Integrate Bochs Parallel debugging

#include "kernel_utils.hpp"

#ifdef THOR_INIT
void serial_transmit(char a);
#else
#include "drivers/serial.hpp"
#endif

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

inline void serial_print(const char* s){
    for(uint64_t i = 0; s[i] != '\0'; ++i){
#ifdef THOR_INIT
        serial_transmit(s[i]);
#else
        serial::transmit(s[i]);
#endif
    }
}

inline void virtual_debug(const char* s){
    if(is_bochs_e9()){
        bochs_print(s);
    } else {
        serial_print(s);
    }
}

#endif
