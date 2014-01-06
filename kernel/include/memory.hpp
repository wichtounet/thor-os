//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MEMORY_H
#define MEMORY_H

#include "stl/types.hpp"

void init_memory_manager();

void* k_malloc(uint64_t bytes);
void k_free(void* block);

/*!
 * \brief Allocates bytes of memory at the given address.
 * \param address The virtual address where memory should be put.
 * \param bytes The number of bytes to allocate.
 * \return The address of the physical allocated memory, 0 indicates failure.
 */
void* k_malloc(uint64_t address, uint64_t bytes);

template<typename T>
T* k_malloc(){
    return reinterpret_cast<T*>(k_malloc(sizeof(T)));
}

uint64_t free_memory();
uint64_t used_memory();
uint64_t allocated_memory();

void memory_debug();

#endif
