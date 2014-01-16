//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "stl/string.hpp"

uint64_t std::parse(const char* it, const char* end){
    int i = end - it - 1;

    uint64_t factor = 1;
    uint64_t acc = 0;

    for(; i >= 0; --i){
        acc += (it[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
}

uint64_t std::parse(const char* str){
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

uint64_t std::parse(const string& str){
    return parse(str.begin(), str.end());
}

uint64_t std::str_len(const char* a){
    uint64_t length = 0;
    while(*a++){
        ++length;
    }
    return length;
}

std::vector<std::string> std::split(const string& s){
    std::vector<string> parts;

    string current(s.size());

    for(char c : s){
        if(c == ' ' && !current.empty()){
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }

    if(!current.empty()){
        parts.push_back(current);
    }

    return std::move(parts);
}
