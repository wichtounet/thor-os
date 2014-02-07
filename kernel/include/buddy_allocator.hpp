//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include "stl/array.hpp"

#include "bitmap.hpp"

//For problems during boot
#include "kernel.hpp"
#include "console.hpp"

template <class T>
inline constexpr T pow(T const& x, size_t n){
    return n > 0 ? x * pow(x, n - 1) : 1;
}

template<size_t Levels, size_t Unit>
struct buddy_allocator {
    static constexpr const size_t levels = Levels;
    static constexpr const size_t max_block = pow(2, levels - 1);

    std::array<static_bitmap, levels> bitmaps;

    size_t first_address;
    size_t last_address;

public:
    void set_memory_range(size_t first, size_t last){
        first_address = first;
        last_address = last;
    }

    template<size_t I>
    void init(size_t words, uint64_t* data){
        bitmaps[I].init(words, data);
    }

    void init(){
        //By default all blocks are free
        for(auto& bitmap : bitmaps){
            bitmap.set_all();
        }
    }

    size_t allocate(size_t pages){
        if(pages > max_block){
            if(pages > max_block * static_bitmap::bits_per_word){
                k_print_line("Virtual block too big");
                suspend_boot();

                //That means we try to allocate more than 33M at the same time
                //probably not a good idea
                //TODO Implement it all the same
                return 0;
            } else {
                auto l = bitmaps.size() - 1;
                auto index = bitmaps[l].free_word();
                auto address = block_start(l, index);

                //TODO check also address + size
                if(address >= last_address){
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
            if(address >= last_address){
                return 0;
            }

            mark_used(l, index);

            return address;
        }
    }

    void free(size_t address, size_t pages){
        if(pages > max_block){
            if(pages > max_block * static_bitmap::bits_per_word){
                k_print_line("Virtual block too big");
                suspend_boot();

                //That means we try to allocate more than 33M at the same time
                //probably not a good idea
                //TODO Implement it all the same
            } else {
                auto l = level(pages);
                auto index = get_block_index(address, l);

                //Mark all bits of the word as free
                for(size_t b = 0; b < static_bitmap::bits_per_word; ++b){
                    mark_free(l, index + b);
                }
            }
        } else {
            auto l = level(pages);
            auto index = get_block_index(address, l);

            mark_free(l, index);
        }
    }

    static size_t level_size(size_t level){
        size_t size = 1;

        for(size_t i = 0; i < level; ++i){
            size *= 2;
        }

        return size;
    }

private:
    static size_t level(size_t pages){
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

    void mark_used(size_t l, size_t index){
        //Mark all sub blocks as taken
        taken_down(l, index);

        //The current level block is not free anymore
        bitmaps[l].unset(index);

        //Mark all up blocks as taken
        taken_up(l, index);
    }

    void mark_free(size_t l, size_t index){
        //Free all sub blocks
        free_down(l, index);

        //Free block at the current level
        bitmaps[l].set(index);

        //Free higher blocks if buddies are free too
        free_up(l, index);
    }

    uintptr_t block_start(size_t l, size_t index) const {
        return first_address + index * level_size(l) * Unit;
    }

    size_t get_block_index(size_t address, size_t l) const {
        return (address - first_address) / (level_size(l) * Unit);
    }

    void taken_down(size_t start_level, size_t index){
        auto start = index * 2;
        auto end = start + 1;

        for(size_t l = start_level; l > 0; --l){
            for(size_t i = start; i <= end; ++i){
                bitmaps[l-1].unset(i);
            }

            start *= 2;
            end = (end * 2) + 1;
        }
    }

    void free_down(size_t start_level, size_t index){
        auto start = index * 2;
        auto end = start + 1;

        for(size_t l = start_level; l > 0; --l){
            for(size_t i = start; i <= end; ++i){
                bitmaps[l-1].set(i);
            }

            start *= 2;
            end = (end * 2) + 1;
        }
    }

    void taken_up(size_t start_level, size_t index){
        for(size_t l = start_level + 1; l < bitmaps.size();  ++l){
            index /= 2;
            bitmaps[l].unset(index);
        }
    }

    void free_up(size_t start_level, size_t index){
        for(size_t l = start_level; l + 1 < bitmaps.size();  ++l){
            size_t buddy_index;
            if(index % 2 == 0){
                buddy_index = index + 1;
            } else {
                buddy_index = index - 1;
            }

            //If buddy is also free, free the block one level higher
            if(bitmaps[l].is_set(buddy_index)){
                index /= 2;
                bitmaps[l+1].set(index);
            } else {
                break;
            }
        }
    }
};

#endif
