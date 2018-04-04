//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include <array.hpp>

#include "bitmap.hpp"
#include "logging.hpp"

/*!
 * \brief Returns the nth power of x
 */
template <class T>
inline constexpr T pow(T const& x, size_t n){
    return n > 0 ? x * pow(x, n - 1) : 1;
}

/*!
 * \brief Buddy allocator system.
 *
 * This is used to allocate physical and virtual memory
 */
template<size_t Levels, size_t Unit>
struct buddy_allocator {
    static constexpr const size_t levels = Levels; ///< The number of levels in the allocator
    static constexpr const size_t max_block = pow(2, levels - 1); ///< The size of the maximum block

    std::array<static_bitmap, levels> bitmaps; ///< The bit maps for each level

    size_t first_address; ///< The first managed address
    size_t last_address;  ///< The last managed address

public:
    /*!
     * \brief Sets the memory range of the allocator
     */
    void set_memory_range(size_t first, size_t last){
        first_address = first;
        last_address = last;
    }

    /*!
     * \brief Initialize the layer I
     * \param words The number of words
     * \param data The memory to use
     */
    template<size_t I>
    void init(size_t words, uint64_t* data){
        bitmaps[I].init(words, data);
    }

    /*!
     * \brief Initialize the allocator
     */
    void init(){
        //By default all blocks are free
        for(auto& bitmap : bitmaps){
            bitmap.set_all();
        }
    }

    /*!
     * \brief Compute the actual memory that will be reserved to
     * allocate for this number of pages.
     */
    size_t necessary_size(size_t pages){
        if(pages <= max_block){
            auto l = level(pages);

            return level_size(l) * Unit;
        } else if(pages <= max_block * static_bitmap::bits_per_word){
            auto l = word_level(pages);

            return word_level_size(l) * Unit;
        } else {
            //TODO Complete allocation for bigger blocks

            return 0;
        }
    }

    /*!
     * \brief Allocate memory for the given amount of pages
     */
    size_t allocate(size_t pages){
        if(pages <= max_block){
            // 1. In the easy case, at most one block of the highest
            //    level will be used

            auto l = level(pages);
            auto index = bitmaps[l].set_bit();

            if(index == static_bitmap::npos){
                logging::logf(logging::log_level::ERROR, "buddy: There is no free bit pages:%u level:%u index:%u\n", pages, l, index);
                return 0;
            }

            auto address = block_start(l, index);

            if(address + level_size(l) * Unit >= last_address){
                logging::logf(logging::log_level::ERROR, "buddy: Address too high pages:%u level:%u index:%u address:%h\n", pages, l, index, address);
                return 0;
            }

            mark_used(l, index);

            return address;
        } else if(pages <= max_block * static_bitmap::bits_per_word){
            // 2. In the more complex case, several contiguous
            //    bits are used to form a bigger block, only
            //    within a single word

            // Select a level for which a whole word can hold the necessary pages
            auto l = word_level(pages);
            auto index = bitmaps[l].set_word();

            if(index == static_bitmap::npos){
                logging::logf(logging::log_level::ERROR, "buddy: There is no free word pages:%u level:%u index:%u\n", pages, l, index);
                return 0;
            }

            auto address = block_start(l, index);

            if(address + word_level_size(l) * Unit >= last_address){
                logging::logf(logging::log_level::ERROR, "buddy: Address too high level:%u index:%u address:%h\n", l, index, address);
                return 0;
            }

            //Mark all bits of the word as used
            for(size_t b = 0; b < static_bitmap::bits_per_word; ++b){
                mark_used(l, index + b);
            }

            return address;
        } else {
            // 3 In the most complex case, several contiguous
            //   words are needed to form a bigger block

            logging::logf(logging::log_level::ERROR, "buddy: Impossible to allocate more than 33M block:%u\n", pages);

            //TODO Implement larger allocation
            return 0;
        }
    }

    /*!
     * \brief Free allocated memory pages
     * \param address The allocated memory
     * \param pages The number of pages to free
     */
    void free(size_t address, size_t pages){
        if(pages > max_block){
            if(pages > max_block * static_bitmap::bits_per_word){
                logging::logf(logging::log_level::ERROR, "buddy: Impossible to free more than 33M block:%u\n", pages);
                //TODO Implement larger allocation
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

    /*!
     * \brief The size of the given level
     */
    static size_t level_size(size_t level){
        return pow(2, level);
    }

    /*!
     * \brief The size of the given level
     */
    static size_t word_level_size(size_t level){
        return static_bitmap::bits_per_word * pow(2, level);
    }

private:
    /*!
     * \brief Returns the level to use for the given amount of pages
     */
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

    size_t word_level(size_t pages) const {
        size_t size = 1;

        for(size_t i = 0; i < levels; ++i){
            if(size * 64 >= pages){
                return i;
            }

            size *= 2;
        }

        return levels;
    }

    /*!
     * \brief Mark the given buddy as used
     * \param l The used level
     * \param index The used index
     */
    void mark_used(size_t l, size_t index){
        //Mark all sub blocks as taken
        taken_down(l, index);

        //The current level block is not free anymore
        bitmaps[l].unset(index);

        //Mark all up blocks as taken
        taken_up(l, index);
    }

    /*!
     * \brief Mark the given buddy as free
     * \param l The freed level
     * \param index The freeed index
     */
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
