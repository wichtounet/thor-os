//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MALLOC_H
#define MALLOC_H

#include "stl/types.hpp"

namespace malloc {

void init();

void* k_malloc(uint64_t bytes);
void k_free(void* block);

template<typename T>
T* k_malloc(){
    return reinterpret_cast<T*>(k_malloc(sizeof(T)));
}

uint64_t allocated_memory();
uint64_t used_memory();
uint64_t free_memory();

void debug();

}

#endif
