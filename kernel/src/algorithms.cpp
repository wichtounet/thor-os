//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "algorithms.hpp"
#include "utils.hpp"

vector<string> split(const string& s){
    vector<string> parts;

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
