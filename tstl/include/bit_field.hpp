//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef BITFIELD_HPP
#define BITFIELD_HPP

#include <types.hpp>

namespace std {

template<typename S, typename T, size_t Position, size_t Size>
struct bit_field {
    S* value;

    bit_field(S* value) : value(value) {}

    T operator*() const {
        return (*value >> Position) & ((1ULL << Size) - 1);
    }

    bit_field& operator=(T new_value){
        S mask = ((1ULL << Size) - 1) << Position;
        *value = (*value & ~mask) | ((new_value << Position) & mask);
        return *this;
    }
};

} //end of namespace std

#endif
