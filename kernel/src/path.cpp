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

path::path(const path& base_path, const std::string& path){
    auto parts = std::split(path, '/');
    names.reserve(base_path.size() + parts.size());

    for(auto& part : base_path){
        names.push_back(part);
    }

    for(auto& part : parts){
        names.push_back(part);
    }
}

// TODO Ideally, the last / should not be used
std::string path::string() const {
    std::string path("/");

    for(auto& part : names){
        path += part;
        path += '/';
    }

    return path;
}

const std::vector<std::string>& path::vec() const {
    return names;
}

void path::invalidate(){
    names.resize(1);
    names[0] = "//";
}

bool path::empty() const {
    return names.empty();
}

bool path::is_root() const {
    return names.empty();
}

bool path::is_valid() const {
    return !(names.size() == 1 && names[0] == "//");
}

size_t path::size() const {
    return names.size();
}

std::string path::base_name() const {
    if(empty()){
        return "";
    } else {
        return names.back();
    }
}

const std::string& path::name(size_t i) const {
    return names[i];
}

const std::string& path::operator[](size_t i) const {
    return names[i];
}

path path::sub_path(size_t i) const {
    path p;
    p.names.resize(size() - i);
    std::copy(p.names.begin(), names.begin() + i, names.end());
    return p;
}

path::iterator path::begin() const {
    return names.begin();
}

path::iterator path::end() const {
    return names.end();
}
