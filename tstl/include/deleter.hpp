//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DELETER_H
#define DELETER_H

namespace std {

template<typename T>
struct default_delete {
    constexpr default_delete() = default;

    constexpr default_delete(const default_delete&) = default;

    void operator()(T* ptr) const {
        static_assert(sizeof(T) > 0, "Type must be complete");
        delete ptr;
    }
};

//Partial specialization for arrays
template<typename T>
struct default_delete<T[]> {
    constexpr default_delete() = default;

    constexpr default_delete(const default_delete&) = default;

    void operator()(T* ptr) const {
        static_assert(sizeof(T) > 0, "Type must be complete");
        delete[] ptr;
    }
};

} //end of namespace std

#endif
