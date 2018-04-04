//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "block_cache.hpp"
#include "kalloc.hpp"
#include "assert.hpp"

void block_cache::init(uint64_t payload_size, uint64_t blocks){
    this->payload_size = payload_size;
    this->blocks = blocks;

    auto block_size = payload_size + 4 * sizeof(uint64_t);

    // Allocate the necessary memory
    this->hash_table = new block_t*[blocks * 2];
    this->blocks_memory = kalloc::k_malloc(blocks * block_size);

    // The table is empty to start with
    for(size_t i = 0; i < blocks * 2; ++i){
        hash_table[i] = nullptr;
    }

    front = reinterpret_cast<block_t*>(blocks_memory);

    block_t* previous = nullptr;

    for(size_t i = 0; i < blocks; ++i){
        auto block = reinterpret_cast<block_t*>(reinterpret_cast<size_t>(blocks_memory) + i * block_size);

        block->key = 0;             // The key
        block->hash_next = nullptr; // The hash pointer

        // The next block in the free list
        if(i < blocks - 1){
            block->free_next = reinterpret_cast<block_t*>(reinterpret_cast<size_t>(blocks_memory) + (i + 1) * block_size);
        } else {
            block->free_next = nullptr;
        }

        // The previous block in the free list
        block->free_prev = previous;

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
        auto* entry = hash_table[key];

        if(entry->key == orig_key){
            return &entry->payload;
        } else {
            while(entry->hash_next){
               entry = entry->hash_next;

               if(entry->key == orig_key){
                    return &entry->payload;
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

    auto* block = front;

    // If the block was used, remove it from the hash table
    if(block->key){
        auto previous_key = block->key % (blocks * 2);

        if(hash_table[previous_key] == block){
            // Use the next block in the bucket list as the first block in the chain
            hash_table[previous_key] = block->hash_next;
        } else {
            auto* entry = block;
            bool found = false;

            while(entry->hash_next){
                auto next = entry->hash_next;

                if(entry->hash_next == block){
                    entry->hash_next = block->hash_next;
                    found = true;
                    break;
                }

                entry = next;
            }

            thor_assert(found, "The hash table chain did not contain the used block");
        }
    }

    // Inserts the block in the hash table

    block->key = orig_key; // Sets the key

    if(!hash_table[key]){
        // Directly sets the head of the table
        block->hash_next = nullptr;
        hash_table[key] = block;
    } else {
        // Insert at the end of the chain

        auto* entry = hash_table[key];

        while(entry->hash_next){
            entry = entry->hash_next;
        }

        entry->hash_next = block;
        block->hash_next = nullptr;
    }

    // Puts the block at the bottom of the free

    front = front->free_next;
    front->free_prev = nullptr;

    rear->free_next = block;
    block->free_prev = rear;
    rear = block;

    return &block->payload;
}
