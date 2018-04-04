//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef LITERALS_HPP
#define LITERALS_HPP

#include <types.hpp>

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
