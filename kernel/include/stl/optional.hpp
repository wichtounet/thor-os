//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIONAL_H
#define OPTIONAL_H

#include "algorithms.hpp"

namespace std {

struct dummy_t {};

template<typename T>
union optional_storage {
    dummy_t dummy;
    T       value;

    constexpr optional_storage() : dummy() {}
    constexpr optional_storage(const T& v) : value(v) {}
    constexpr optional_storage(T&& v) : value(v) {}

    optional_storage& operator=(const T& v){
        value = v;
    }

    optional_storage& operator=(T&& v){
        value = v;
    }

    ~optional_storage(){}
};

template<typename T>
class optional {
    bool initialized;
    optional_storage<T> storage;

public:
    constexpr optional() : initialized(false), storage() {}
    constexpr optional(const T& value) : initialized(true), storage(value) {}

    optional(const optional& rhs) : initialized(rhs.initialized) {
        if(rhs.initialized){
            storage.value = rhs.storage.value;
        }
    }

    optional(optional&& rhs) : initialized(rhs.initialized) {
        if(rhs.initialized){
            storage.value = std::move(rhs.storage.value);
        }
    }

    constexpr T const& operator*(){
        return storage.value;
    }

    ~optional(){
        if(initialized){
            storage.value.~T();
        }
    }

    constexpr const T* operator->() const {
        return &storage.value;
    }

    T* operator->() {
        return &storage.value;
    }

    T& operator*(){
        return storage.value;
    }

    constexpr operator bool() const {
        return initialized;
    }
};

} //end of namespace std

#endif
