//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef BLOCK_CACHE_HPP
#define BLOCK_CACHE_HPP

#include <types.hpp>

struct block_t {
    uint64_t key;
    block_t* hash_next;
    block_t* free_next;
    block_t* free_prev;
    char payload;
} __attribute__((packed));

struct block_cache {
    uint64_t payload_size;
    uint64_t blocks;

    void* blocks_memory;

    block_t** hash_table;

    block_t* front;
    block_t* rear;

    void init(uint64_t payload_size, uint64_t blocks);

    char* block_if_present(uint16_t device, uint64_t sector);
    char* block_if_present(uint64_t key);

    char* block(uint16_t device, uint64_t sector, bool& valid);
    char* block(uint64_t key, bool& valid);
};

#endif
