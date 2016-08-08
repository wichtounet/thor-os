//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "block_cache.hpp"
#include "kalloc.hpp"
#include "assert.hpp"

void block_cache::init(uint64_t payload_size, uint64_t blocks){
    this->payload_size = payload_size;
    this->blocks = blocks;

    auto block_size = payload_size + 4 * sizeof(uint64_t);

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

        // The next block in the free list
        if(i < blocks - 1){
            block[2] = reinterpret_cast<size_t>(blocks_memory) + (i + 1) * block_size;
        } else {
            block[2] = 0;
        }

        // The previous block in the free list
        block[3] = reinterpret_cast<size_t>(previous);

        previous = block;
    }

    rear = previous;
}

char* block_cache::block_if_present(uint16_t device, uint64_t sector){
    return block_if_present((uint64_t(device) << 16) + sector);
}

char* block_cache::block_if_present(uint64_t orig_key){
    auto key = orig_key % (blocks * 2);

    if(hash_table[key]){
        auto* entry = static_cast<uint64_t*>(hash_table[key]);

        if(entry[0] == orig_key){
            return reinterpret_cast<char*>(reinterpret_cast<size_t>(hash_table[key]) + 4 * sizeof(uint64_t));
        } else {
            while(entry[1]){
               entry = reinterpret_cast<uint64_t*>(entry[1]);

               if(entry[0] == orig_key){
                   return reinterpret_cast<char*>(reinterpret_cast<size_t>(entry) + 4 * sizeof(uint64_t));
               }
            }
        }
    }

    return nullptr;
}

char* block_cache::block(uint16_t device, uint64_t sector, bool& valid){
    return block((uint64_t(device) << 16) + sector, valid);
}

char* block_cache::block(uint64_t orig_key, bool& valid){
    auto key = orig_key % (blocks * 2);

    // First, try to get it directly from the hash table

    auto direct = block_if_present(orig_key);

    if(direct){
        valid = true;
        return direct;
    }

    // At this point, we will allocate a new block
    valid = false;

    auto* block = reinterpret_cast<uint64_t*>(front);

    // If the block was used, remove it from the hash table
    if(block[0]){
        auto previous_key = block[0] % (blocks * 2);

        if(hash_table[previous_key] == block){
            // Use the next block in the bucket list as the first block in the chain
            hash_table[previous_key] = reinterpret_cast<uint64_t*>(block[1]);
        } else {
            auto* entry = block;
            bool found = false;

            while(entry[1]){
                auto next = entry[1];

                if(entry[1] == reinterpret_cast<size_t>(block)){
                    entry[1] = block[1];
                    found = true;
                    break;
                }

                entry = reinterpret_cast<uint64_t*>(next);
            }

            thor_assert(found, "The hash table chain did not contain the used block");
        }
    }

    // Inserts the block in the hash table

    block[0] = orig_key; // Sets the key

    if(!hash_table[key]){
        // Directly sets the head of the table
        block[1] = 0;
        hash_table[key] = block;
    } else {
        // Insert at the end of the chain

        auto* entry = hash_table[key];

        while(entry[1]){
            entry = reinterpret_cast<uint64_t*>(entry[1]);
        }

        entry[1] = reinterpret_cast<uint64_t>(block);
        block[1] = 0;
    }

    // Puts the block at the bottom of the free

    front = reinterpret_cast<void*>(reinterpret_cast<uint64_t*>(front)[2]);
    reinterpret_cast<uint64_t*>(front)[3] = 0;

    reinterpret_cast<uint64_t*>(rear)[2] = reinterpret_cast<uint64_t>(block);
    block[3] = reinterpret_cast<uint64_t>(rear);
    rear = block;

    return reinterpret_cast<char*>(reinterpret_cast<uint64_t>(block) + 4 * sizeof(uint64_t));
}
