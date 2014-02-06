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

    size_t free_word() const {
        for(size_t w = 0; w < words; ++w){
            if(data[w] == ~static_cast<data_type>(0)){
                return w * bits_per_word;
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

size_t level_size(size_t level){
    size_t size = 1;

    for(size_t i = 0; i < level; ++i){
        size *= 2;
    }

    return size;
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

void taken_down(size_t level, size_t index){
    if(level == 0){
        return;
    }

    bitmaps[level-1].unset(index * 2);
    bitmaps[level-1].unset(index * 2 + 1);

    taken_down(level - 1, index * 2);
    taken_down(level - 1, index * 2 + 1);
}

void free_down(size_t level, size_t index){
    if(level == 0){
        return;
    }

    bitmaps[level-1].set(index * 2);
    bitmaps[level-1].set(index * 2 + 1);

    taken_down(level - 1, index * 2);
    taken_down(level - 1, index * 2 + 1);
}

void taken_up(size_t level, size_t index){
    if(level == bitmaps.size() - 1){
        return;
    }

    bitmaps[level+1].unset(index / 2);
    taken_up(level+1, index / 2);
}

void free_up(size_t level, size_t index){
    if(level == bitmaps.size() - 1){
        return;
    }

    size_t buddy_index;
    if(index % 2 == 0){
        buddy_index = index + 1;
    } else {
        buddy_index = index - 1;
    }

    //If buddy is also free, free the block one level higher
    if(bitmaps[level].is_set(buddy_index)){
        bitmaps[level+1].set(index / 2);
        taken_up(level+1, index / 2);
    }
}

uintptr_t block_start(size_t level, size_t index){
    return first_virtual_address + index * level_size(level) * unit;
}

size_t get_block_index(size_t address, size_t level){
    return (address - first_virtual_address) / (level_size(level) * unit);
}

void mark_used(size_t l, size_t index){
    //The current level block is not free anymore
    bitmaps[l].unset(index);

    //Mark all sub blocks as taken
    taken_down(l, index);

    //Mark all up blocks as taken
    taken_up(l, index);
}

size_t virtual_allocator::allocate(size_t pages){
    //TODO Return 0 if not enough pages

    allocated_pages += pages;

    if(pages > max_block){
        if(pages > max_block * static_bitmap::bits_per_word){
            //That means we try to allocate more than 33M at the same time
            //probably not a good idea
            //TODO Implement it all the same
            return 0;
        } else {
            auto l = bitmaps.size() - 1;
            auto index = bitmaps[l].free_word();
            auto address = block_start(l, index);

            //TODO check also address + size
            if(address >= last_virtual_address){
                return 0;
            }

            //Mark all bits of the word as used
            for(size_t b = 0; b < static_bitmap::bits_per_word; ++b){
                mark_used(l, index + b);
            }

            return address;
        }
    } else {
        auto l = level(pages);
        auto index = bitmaps[l].free_bit();
        auto address = block_start(l, index);

        //TODO check also address + size
        if(address >= last_virtual_address){
            return 0;
        }

        mark_used(l, index);

        return address;
    }
}

void virtual_allocator::free(size_t address, size_t pages){
    allocated_pages -= pages;

    if(pages > max_block){
        //TODO Special algorithm for big pages
    } else {
        auto l = level(pages);
        auto index = get_block_index(address, l);

        //Free block
        bitmaps[l].set(index);

        //Free all sub blocks
        free_down(l, index);

        //Free higher blocks if buddies are free too
        free_up(l, index);
    }
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
