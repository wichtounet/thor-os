//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "vfs/path.hpp"

#include "assert.hpp"

path::path(){
    //Nothing to init
}

path::path(const std::string& path){
    if(path[0] == '/'){
        names.push_back("/");
    }

    std::split_append(path, names, '/');
}

path::path(const path& base_path, const std::string& p){
    thor_assert(p.empty() || p[0] != '/', "Impossible to add absolute path to another path");

    auto parts = std::split(p, '/');
    names.reserve(names.size() + base_path.size() + parts.size());

    std::copy(base_path.begin(), base_path.end(), std::back_inserter(names));
    std::copy(parts.begin(), parts.end(), std::back_inserter(names));
}

path::path(const path& base_path, const path& p){
    thor_assert(p.is_relative(), "Impossible to add absolute path to another path");

    names.reserve(names.size() + base_path.size() + p.size());

    std::copy(base_path.begin(), base_path.end(), std::back_inserter(names));
    std::copy(p.begin(), p.end(), std::back_inserter(names));
}

// TODO Ideally, the last / should not be used
std::string path::string() const {
    std::string str_path;

    size_t i = 0;

    if(is_absolute()){
        str_path = "/";
        i = 1;
    }

    std::string comma;
    for(; i < names.size(); ++i){
        str_path += comma;
        str_path += names[i];

        comma = "/";
    }

    return str_path;
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
    return names.size() == 1 && names[0] == "/";
}

bool path::is_valid() const {
    return !names.empty() && !(names.size() == 1 && names[0] == "//");
}

bool path::is_sub_root() const {
    return is_absolute() && names.size() == 2;
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

std::string path::root_name() const {
    if(empty()){
        return "";
    } else {
        return names.front();
    }
}

std::string path::sub_root_name() const {
    if(size() < 2){
        return "";
    } else {
        return names[1];
    }
}

bool path::is_absolute() const {
    return names.size() && names[0] == "/";
}

bool path::is_relative() const {
    return names.size() && names[0] != "/";
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
    std::copy(names.begin() + i, names.end(), p.names.begin());
    return p;
}

path path::branch_path() const {
    if(empty()){
        return *this;
    }

    if(is_root()){
        return *this;
    }

    if(is_relative() && size() == 1){
        return *this;
    }

    path p;
    p.names.resize(size() - 1);
    std::copy(names.begin(), names.end() - 1, p.names.begin());
    return p;
}

path::iterator path::begin() const {
    return names.begin();
}

path::iterator path::end() const {
    return names.end();
}

bool path::operator==(const path& p) const {
    return names == p.names;
}

bool path::operator!=(const path& p) const {
    return names != p.names;
}

path operator/(const path& lhs, const path& rhs){
    return {lhs, rhs};
}

path operator/(const path& lhs, const std::string& rhs){
    return {lhs, rhs};
}

path operator/(const std::string& lhs, const path& rhs){
    return {path(lhs), rhs};
}
