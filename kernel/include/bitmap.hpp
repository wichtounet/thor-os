//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BITMAP_H
#define BITMAP_H

#include "assert.hpp"

struct static_bitmap {
    typedef uint64_t data_type;

    static constexpr const size_t bits_per_word = sizeof(data_type) * 8;

    size_t words;
    data_type* data;

    template<typename Array>
    void init(Array& array){
        words = array.size();
        data = array.data();
    }

    void init(size_t w, data_type* d){
        words = w;
        data = d;
    }

    static constexpr size_t word_offset(size_t bit){
        return bit / bits_per_word;
    }

    static constexpr size_t bit_offset(size_t bit){
        return bit % bits_per_word;
    }

    static constexpr data_type bit_mask(size_t bit){
        return static_cast<data_type>(1) << bit_offset(bit);
    }

    void clear_all(){
        for(size_t i = 0; i < words; ++i){
            data[i] = 0;
        }
    }

    void set_all(){
        for(size_t i = 0; i < words; ++i){
            data[i] = ~static_cast<data_type>(0);
        }
    }

    size_t free_bit() const {
        for(size_t w = 0; w < words; ++w){
            if(data[w] > 0){
                for(size_t b = 0; b < bits_per_word; ++b){
                    if(data[w] & bit_mask(b)){
                        return w * bits_per_word + b;
                    }
                }
            }
        }

        thor_unreachable("static_bitmap has not free bit");
    }

    size_t free_word() const {
        for(size_t w = 0; w < words; ++w){
            if(data[w] == ~static_cast<data_type>(0)){
                return w * bits_per_word;
            }
        }

        thor_unreachable("static_bitmap has no free word");
    }

    bool is_set(size_t bit) const {
        return data[word_offset(bit)] & bit_mask(bit);
    }

    void set(size_t bit){
        data[word_offset(bit)] |= bit_mask(bit);
    }

    void unset(size_t bit){
        data[word_offset(bit)] &= ~bit_mask(bit);
    }
};

#endif
