//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "stl/array.hpp"

#include "virtual_allocator.hpp"
#include "paging.hpp"

namespace {

constexpr const size_t virtual_start = paging::virtual_paging_start + (paging::physical_memory_pages * paging::PAGE_SIZE);
constexpr const size_t first_virtual_address = virtual_start % 0x100000 == 0 ? virtual_start : (virtual_start / 0x100000 + 1) * 0x100000;
constexpr const size_t last_virtual_address = virtual_allocator::kernel_virtual_size;
constexpr const size_t managed_space = last_virtual_address - first_virtual_address;
constexpr const size_t unit = paging::PAGE_SIZE;

size_t next_virtual_address = first_virtual_address;
size_t allocated_pages = first_virtual_address / paging::PAGE_SIZE;

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

        //TODO Use an assert here
        return 0;
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

constexpr size_t array_size(int block){
    return (managed_space / (block * unit) + 1) / (sizeof(uint64_t) * 8) + 1;
}

static constexpr const size_t levels = 8;
static constexpr const size_t max_block = 128;

std::array<uint64_t, array_size(1)> data_bitmap_1;
std::array<uint64_t, array_size(2)> data_bitmap_2;
std::array<uint64_t, array_size(4)> data_bitmap_4;
std::array<uint64_t, array_size(8)> data_bitmap_8;
std::array<uint64_t, array_size(16)> data_bitmap_16;
std::array<uint64_t, array_size(32)> data_bitmap_32;
std::array<uint64_t, array_size(64)> data_bitmap_64;
std::array<uint64_t, array_size(128)> data_bitmap_128;

std::array<static_bitmap, levels> bitmaps;

size_t level(size_t pages){
    if(pages > 64){
        return 7;
    } else if(pages > 32){
        return 6;
    } else if(pages > 16){
        return 5;
    } else if(pages > 8){
        return 4;
    } else if(pages > 4){
        return 3;
    } else if(pages > 2){
        return 2;
    } else if(pages > 1){
        return 1;
    } else {
        return 0;
    }
}

size_t get_free_block_index(size_t level){
    auto& bitmap = bitmaps[level];
    return bitmap.free_bit();
}

} //end of anonymous namespace

void virtual_allocator::init(){
    //Give room to the bitmaps
    bitmaps[0].init(data_bitmap_1);
    bitmaps[1].init(data_bitmap_2);
    bitmaps[2].init(data_bitmap_4);
    bitmaps[3].init(data_bitmap_8);
    bitmaps[4].init(data_bitmap_16);
    bitmaps[5].init(data_bitmap_32);
    bitmaps[6].init(data_bitmap_64);
    bitmaps[7].init(data_bitmap_128);

    //By default all blocks are free
    for(auto& bitmap : bitmaps){
        bitmap.set_all();
    }
}

size_t virtual_allocator::allocate(size_t pages){
    allocated_pages += pages;

    if(pages > max_block){
        //TODO Special algorithm for big pages
    } else {
        auto l = level(pages);
        auto index = get_free_block_index(l);
    }


    auto address = next_virtual_address;
    next_virtual_address += pages * paging::PAGE_SIZE;
    return address;
}

size_t virtual_allocator::available(){
    return kernel_virtual_size;
}

size_t virtual_allocator::allocated(){
    return allocated_pages * paging::PAGE_SIZE;
}

size_t virtual_allocator::free(){
    return available() - allocated();
}
