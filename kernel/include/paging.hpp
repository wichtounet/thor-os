//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PAGING_H
#define PAGING_H

#include "stl/types.hpp"

namespace paging {

constexpr const int PAGE_SIZE = 4096;

template<typename T>
constexpr bool page_aligned(T* addr){
    return !(reinterpret_cast<uintptr_t>(addr) & (paging::PAGE_SIZE - 1));
}

template<typename T>
constexpr T* page_align(T* addr){
    return reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(addr) / paging::PAGE_SIZE) * paging::PAGE_SIZE);
}

void* physical_address(void* virt);
bool page_present(void* virt);
bool page_free_or_set(void* virt, void* physical);

bool identity_map(void* virt);
bool identity_map(void* virt, size_t pages);

bool map(void* virt, void* physical);
bool map(void* virt, void* physical, size_t pages);

bool unmap(void* virt);
bool unmap(void* virt, size_t pages);

} //end of namespace paging

#endif
