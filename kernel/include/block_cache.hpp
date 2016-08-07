//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BLOCK_CACHE_HPP
#define BLOCK_CACHE_HPP

#include <types.hpp>

struct block_cache {
    uint64_t payload_size;
    uint64_t blocks;

    uint64_t** hash_table;
    void* blocks_memory;

    void* front;
    void* rear;

    void init(uint64_t payload_size, uint64_t blocks);

    char* block(uint16_t device, uint64_t sector);
};

#endif
