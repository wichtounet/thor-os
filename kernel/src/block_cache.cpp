//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "block_cache.hpp"
#include "kalloc.hpp"

void block_cache::init(uint64_t payload_size, uint64_t blocks){
    this->payload_size = payload_size;
    this->blocks = blocks;

    auto block_size = payload_size + 4 * 8;

    this->hash_table = new uint64_t*[blocks * 2];
    this->blocks_memory = kalloc::k_malloc(blocks * block_size);

    for(size_t i = 0; i < blocks * 2; ++i){
        hash_table[i] = nullptr;
    }

    front = blocks_memory;

    void* previous = nullptr;

    for(size_t i = 0; i < blocks; ++i){
        auto block = reinterpret_cast<uint64_t*>(reinterpret_cast<size_t>(blocks_memory) + i * block_size);

        block[0] = 0; // The key
        block[1] = 0; // The hash pointer
        block[2] = reinterpret_cast<size_t>(previous); // The previous block in the free list

        // The next block in the free list
        if(i < blocks - 1){
            block[3] = reinterpret_cast<size_t>(blocks_memory) + (i + 1) * block_size;
        } else {
            block[3] = 0;
        }

        previous = block;
    }

    rear = previous;
}

char* block_cache::block(uint16_t device, uint64_t sector){
    auto key = (uint64_t(device) << 16) + sector % (blocks * 2);

    if(hash_table[key]){
        auto* entry = static_cast<uint64_t*>(hash_table[key]);

        if(entry[0] == key){
            return reinterpret_cast<char*>(reinterpret_cast<size_t>(hash_table[key]) + 4 * 8);
        } else {
            while(entry[1]){
               entry = reinterpret_cast<uint64_t*>(entry[1]);

               if(entry[0] == key){
                   return reinterpret_cast<char*>(reinterpret_cast<size_t>(entry) + 4 * 8);
               }
            }
        }
    }

    // At this point, the block is not present

    //TODO Continue
}
