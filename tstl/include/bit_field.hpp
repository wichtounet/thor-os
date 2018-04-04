//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef BITFIELD_HPP
#define BITFIELD_HPP

#include <types.hpp>

namespace std {

/*!
 * \brief Helper to handle a value as a bit field.
 * \param S The type of the source value
 * \param T The type of the bit value
 * \param Position The starting position from the right (LSB)
 * \param Size The number of bits
 */
template<typename S, typename T, size_t Position, size_t Size>
struct bit_field {
    /*!
     * \brief Construct a bit field around the given value
     */
    bit_field(S* value) : value(value) {}

    /*!
     * \brief Extract the value of the bit field
     */
    T get() const {
        return (*value >> Position) & ((1ULL << Size) - 1);
    }

    /*!
     * \brief Extract the value of the bit field
     */
    T operator*() const {
        return get();
    }

    /*!
     * \brief Assign a new value to the bit field
     */
    bit_field& operator=(T new_value_real){
        S new_value(new_value_real);

        size_t mask = ((S(1) << Size) - 1) << Position;
        *value = (*value & ~mask) | ((new_value << Position) & mask);
        return *this;
    }

private:
    S* value; ///< Pointer to the source value
};

} //end of namespace std

#endif
