//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "vfs/path.hpp"

#include "assert.hpp"

namespace {

void update_positions(std::string& base, std::small_vector<path::position_t>& positions){
    positions.clear();

    if(base.empty()){
        return;
    }

    for(int i = base.size() - 2; i >= 0; --i){
        if(base[i] == '/' && base[i+1] == '/'){
            base.erase(size_t(i+1));
        }
    }

    // Canonicalize this
    // TODO Ideally we should handle that before to avoid a possible
    // second allocation here!
    if(base.back() != '/'){
        base += '/';
    }

    for(size_t i = 0; i < base.size(); ++i){
        if(base[i] == '/'){
            positions.push_back(i);
        }
    }
}

} // end of anonymous namespaces

path::path(){
    //Nothing to init
}

path::path(std::string_view path) : base(path.begin(), path.end()) {
    thor_assert(path.size(), "Invalid base path");

    update_positions(base, positions);
}

path::path(const path& base_path, std::string_view p) : base(base_path.base) {
    thor_assert(p.empty() || p[0] != '/', "Impossible to add absolute path to another path");

    //TODO Add support for string::operator+=(string_view)
    base += std::string(p.begin(), p.end());

    update_positions(base, positions);
}

path::path(const path& base_path, const path& p) : base(base_path.base) {
    thor_assert(p.is_relative(), "Impossible to add absolute path to another path");

    base += p.base;

    update_positions(base, positions);
}

path& path::operator=(std::string_view rhs){
    base = rhs;

    update_positions(base, positions);

    return *this;
}

std::string_view path::string() const {
    return {base.c_str(), base.size()};
}

void path::invalidate(){
    base = "//";
    positions.clear();
}

bool path::empty() const {
    return base.empty();
}

bool path::is_root() const {
    return base == "/";
}

bool path::is_valid() const {
    return !base.empty() && base != "//";
}

bool path::is_sub_root() const {
    return is_absolute() && positions.size() == 2;
}

size_t path::size() const {
    return positions.size();
}

std::string_view path::base_name() const {
    if (empty()) {
        return "";
    }

    if (is_root()) {
        return "/";
    }

    if (is_absolute()) {
        return {base.begin() + positions[size() - 2] + 1, base.size() - 1 - static_cast<size_t>(positions[size() - 2] + 1)};
    }

    if (size() == 1) {
        return {base.begin(), base.size() - 1};
    } else {
        return {base.begin() + positions[size() - 2] + 1, base.size() - 1 - static_cast<size_t>(positions[size() - 2] + 1)};
    }
}

std::string_view path::root_name() const {
    if(empty()){
        return "";
    }

    if(is_absolute()){
        return "/";
    }

    return {base.begin(), positions.front()};
}

std::string_view path::sub_root_name() const {
    thor_assert(is_absolute(), "sub_root_name() does not make sense on relative path");

    if(size() < 2){
        return "";
    }

    return {base.begin() + 1, static_cast<size_t>(positions[1]) - 1};
}

bool path::is_absolute() const {
    return !base.empty() && base[0] == '/';
}

bool path::is_relative() const {
    return !is_absolute();
}

std::string_view path::name(size_t i) const {
    if (is_absolute()) {
        if (i == 0) {
            return "/";
        }

        return {base.begin() + positions[i - 1] + 1, size_t(positions[i] - 1) - positions[i - 1]};
    }

    if (i == 0) {
        return {base.begin(), positions[i]};
    }

    return {base.begin() + positions[i - 1] + 1, size_t(positions[i] - 1) - positions[i - 1]};
}

std::string_view path::operator[](size_t i) const {
    return name(i);
}

path path::sub_path(size_t i) const {
    if (i == 0) {
        return *this;
    }

    path p;

    if (is_absolute()) {
        if (i == size()) {
            return p;
        }

        p.base.assign(base.begin() + positions[i - 1] + 1, base.end());
    } else {
        if (i == size()) {
            return p;
        }

        p.base.assign(base.begin() + positions[i - 1] + 1, base.end());
    }

    update_positions(p.base, p.positions);

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

    if(is_relative()){
        p.base.assign(base.begin(), base.begin() + positions[size() - 2]);
    } else {
        p.base.assign(base.begin(), base.begin() + positions[size() - 2]);
    }

    update_positions(p.base, p.positions);

    return p;
}

bool path::operator==(const path& p) const {
    return base == p.base;
}

bool path::operator!=(const path& p) const {
    return base != p.base;
}

bool path::operator==(std::string_view p) const {
    // Note: This is highly inefficient
    path rhs(p);
    return *this == rhs;
}

bool path::operator!=(std::string_view p) const {
    return !(*this == p);
}

path operator/(const path& lhs, const path& rhs){
    return {lhs, rhs};
}

path operator/(const path& lhs, std::string_view rhs){
    return {lhs, rhs};
}

path operator/(const path& lhs, const char* rhs){
    return {lhs, rhs};
}

path operator/(std::string_view lhs, const path& rhs){
    return {path(lhs), rhs};
}
