//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LITERALS_HPP
#define LITERALS_HPP

#include "stl/types.hpp"

static_assert(sizeof(size_t) == sizeof(unsigned long long), "Unmatching sizes for literals");

inline constexpr size_t operator"" _GiB (unsigned long long n){
    return n * 1024 * 1024 * 1024;
}

inline constexpr size_t operator"" _MiB (unsigned long long n){
    return n * 1024 * 1024;
}

inline constexpr size_t operator"" _KiB (unsigned long long n){
    return n * 1024;
}

#endif
