//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "utils.hpp"
#include "string.hpp"

int memcmp(const void* s1, const void* s2, size_t n){
    auto p1 = static_cast<const unsigned char*>(s1);
    auto p2 = static_cast<const unsigned char*>(s2);

    while(n--){
        if( *p1 != *p2 ){
            return *p1 - *p2;
        } else {
            p1++;
            p2++;
        }
    }

    return 0;
}

void memcopy(void* destination, const void* source, size_t n){
    auto dest = static_cast<unsigned char*>(destination);
    auto src = static_cast<const unsigned char*>(source);

    --dest;
    --src;

    while(n--){
        *++dest = *++src;
    }
}

uint64_t parse(const char* it, const char* end){
    int i = end - it - 1;

    uint64_t factor = 1;
    uint64_t acc = 0;

    for(; i >= 0; --i){
        acc += (it[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
}

uint64_t parse(const char* str){
    int i = 0;

    const char* it = str;
    while(*++it){
        ++i;
    }

    uint64_t factor = 1;
    uint64_t acc = 0;

    for(; i >= 0; --i){
        acc += (str[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
}

uint64_t parse(const string& str){
    return parse(str.begin(), str.end());
}

uint64_t str_len(const char* a){
    uint64_t length = 0;
    while(*a++){
        ++length;
    }
    return length;
}
