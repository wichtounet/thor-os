//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UNIQUE_PTR_H
#define UNIQUE_PTR_H

#include <tuple.hpp>

#include "thor.hpp"

namespace std {

template<typename T>
struct default_delete {
    default_delete() {}

    template<typename Up>
    default_delete(const default_delete<Up>&) {}

    void operator()(T* ptr) const {
        static_assert(sizeof(T) > 0, "Type must be complete");
        delete ptr;
    }
};

template <typename T, typename D = default_delete<T>>
class unique_ptr {
public:
    typedef T* pointer_type;
    typedef T element_type;
    typedef D deleter_type;

private:
    typedef tuple<pointer_type, deleter_type> data_impl;

    data_impl _data;

    //pointer_type pointer;
    //deleter_type deleter;

public:
    unique_ptr() : _data(make_tuple(pointer_type(), deleter_type())) {}

    explicit unique_ptr(pointer_type p) : _data(make_tuple(p, deleter_type())) {}

    unique_ptr(unique_ptr&& u) : _data(make_tuple(u.release(), u.get_deleter())) {}
    unique_ptr& operator=(unique_ptr&& u){
        reset(u.release());
        return *this;
    }

    ~unique_ptr(){
        reset();
    }

    // Disable copy
    unique_ptr(const unique_ptr& rhs) = delete;
    unique_ptr& operator=(const unique_ptr& rhs) = delete;

    element_type& operator*() const {
        return *get();
    }

    pointer_type operator->() const {
        return get();
    }

    pointer_type get() const {
        return std::get<0>(_data);
    }

    deleter_type get_deleter() const {
        return std::get<1>(_data);
    }

    operator bool() const {
        return get();
    }

    pointer_type release(){
        pointer_type p = get();
        std::get<0>(_data) = nullptr;
        return p;
    }

    void reset(pointer_type p = pointer_type()){
        if(get() != p){
            get_deleter()(get());
            std::get<0>(_data) = nullptr;
        }
    }
};

static_assert(sizeof(unique_ptr<long>) == sizeof(long), "unique_ptr must have zero overhead with default deleter");

} //end of namespace std

#endif
