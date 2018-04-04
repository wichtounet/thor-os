//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef BITMAP_H
#define BITMAP_H

#include "assert.hpp"

/*!
 * \brief A static bitmap.
 *
 * This bitmap cannot be extended. The data must be provided via one of its init functions.
 */
struct static_bitmap {
    using data_type = uint64_t; ///< The word type

    static constexpr const size_t bits_per_word = sizeof(data_type) * 8; ///< The number of bits stored in each word
    static constexpr const size_t npos = 18446744073709551615ULL;        ///< Special number indicating an error

    template<typename Array>
    void init(Array& array){
        words = array.size();
        data = array.data();
    }

    void init(size_t w, data_type* d){
        words = w;
        data = d;
    }

    /*!
     * \brief Returns the word in which the bit is
     */
    static constexpr size_t word_offset(size_t bit){
        return bit / bits_per_word;
    }

    /*!
     * \brief Returns the offset of the bit inside its word
     */
    static constexpr size_t bit_offset(size_t bit){
        return bit % bits_per_word;
    }

    /*
     * \brief Constructs a bit mask for the given bit
     */
    static constexpr data_type bit_mask(size_t bit){
        return static_cast<data_type>(1) << bit_offset(bit);
    }

    /*!
     * \brief Clear the complete bit map (set everything to zero)
     */
    void clear_all(){
        for(size_t i = 0; i < words; ++i){
            data[i] = 0;
        }
    }

    /*!
     * \brief Set the complete bit map (set everything to one)
     */
    void set_all(){
        for(size_t i = 0; i < words; ++i){
            data[i] = ~static_cast<data_type>(0);
        }
    }

    /*!
     * \brief Returns the first set bit in the bit map
     */
    size_t set_bit() const {
        for(size_t w = 0; w < words; ++w){
            if(data[w]){
                for(size_t b = 0; b < bits_per_word; ++b){
                    if(data[w] & bit_mask(b)){
                        return w * bits_per_word + b;
                    }
                }
            }
        }

        return npos;
    }

    /*!
     * \brief Returns the first set word in the bit map
     */
    size_t set_word() const {
        for(size_t w = 0; w < words; ++w){
            if(data[w] == ~static_cast<data_type>(0)){
                return w * bits_per_word;
            }
        }

        return npos;
    }

    /*!
     * \brief Indicates if the given bit is set
     */
    bool is_set(size_t bit) const {
        return data[word_offset(bit)] & bit_mask(bit);
    }

    /*!
     * \brief Sets the given bit to 1
     */
    void set(size_t bit){
        data[word_offset(bit)] |= bit_mask(bit);
    }

    /*!
     * \brief clear the given bit to 1
     */
    void unset(size_t bit){
        data[word_offset(bit)] &= ~bit_mask(bit);
    }

private:
    size_t words; ///< Number of words used in the bitmap
    data_type* data; ///< The data storage
};

#endif
