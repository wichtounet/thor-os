//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "vfs/path.hpp"

path::path(){
    //Nothing to init
}

path::path(const std::string& path){
    auto parts = std::split(path, '/');
    names.reserve(parts.size());
    for(auto& part : parts){
        names.push_back(part);
    }
}

bool path::empty() const {
    return names.empty();
}
