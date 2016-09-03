//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INITIALIZER_LIST_H
#define INITIALIZER_LIST_H

namespace std {

template <typename T>
struct initializer_list {
    const T* first;
    size_t _size;

    constexpr initializer_list(const T* __b, size_t __s) noexcept : first(__b), _size(__s) {
        // Nothing else to init
    }

public:
    using value_type      = T;
    using reference       = const T&;
    using const_reference = const T&;
    using size_type       = size_t;
    using iterator        = const T*;
    using const_iterator  = const T*;

    constexpr initializer_list() noexcept : first(nullptr), _size(0) {
        // Nothing else to init
    }

    constexpr size_t size() const noexcept {
        return _size;
    }

    constexpr const T* begin() const noexcept {
        return first;
    }

    constexpr const T* end() const noexcept {
        return first + _size;
    }
};

template <typename T>
inline constexpr const T* begin(initializer_list<T> list) noexcept {
    return list.begin();
}

template <typename T>
inline constexpr const T* end(initializer_list<T> list) noexcept {
    return list.end();
}

} //end of namespace std

#endif
