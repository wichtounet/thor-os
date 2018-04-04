//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef MALLOC_H
#define MALLOC_H

#include <types.hpp>

namespace kalloc {

void init();
void finalize();

void* k_malloc(uint64_t bytes);
void k_free(void* block);

template<typename T>
T* k_malloc(){
    return reinterpret_cast<T*>(k_malloc(sizeof(T)));
}

uint64_t allocated_memory();
uint64_t used_memory();
uint64_t allocations();
uint64_t free_memory();

void debug();

}

#endif
