//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef CSTRING_H
#define CSTRING_H

#include <types.hpp>

namespace std {

inline uint64_t str_len(const char* a){
    uint64_t length = 0;
    while(*a++){
        ++length;
    }
    return length;
}

} //end of namespace std

#endif
