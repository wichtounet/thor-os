//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PAGING_H
#define PAGING_H

#include "stl/types.hpp"

namespace paging {

constexpr const int PAGE_SIZE = 4096;

//Flags
constexpr const uint8_t PRESENT = 0x1;
constexpr const uint8_t WRITE = 0x2;
constexpr const uint8_t USER = 0x4;
constexpr const uint8_t WRITE_THROUGH = 0x8;
constexpr const uint8_t CACHE_DISABLED = 0x10;
constexpr const uint8_t ACCESSED= 0x20;

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

bool identity_map(void* virt, uint8_t flags = PRESENT | WRITE);
bool identity_map_pages(void* virt, size_t pages, uint8_t flags = PRESENT | WRITE);

bool map(void* virt, void* physical, uint8_t flags = PRESENT | WRITE);
bool map_pages(void* virt, void* physical, size_t pages, uint8_t flags = PRESENT | WRITE);

bool unmap(void* virt);
bool unmap_pages(void* virt, size_t pages);

} //end of namespace paging

#endif
