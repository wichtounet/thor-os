//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "paging.hpp"
#include "types.hpp"

namespace {

typedef uint64_t* page_entry;
typedef page_entry* pt_t;
typedef pt_t* pdt_t;
typedef pdt_t* pdpt_t;
typedef pdpt_t* pml4t_t;

} //end of anonymous namespace

bool paging::identity_map(void* physical){
    //The address must be page-aligned
    if(reinterpret_cast<uintptr_t>(physical) % PAGE_SIZE != 0){
        return false;
    }

    //Find the correct indexes inside the paging table for the physical address
    auto table = (reinterpret_cast<uintptr_t>(physical) >> 12) & 0x1FF;
    auto directory = (reinterpret_cast<uintptr_t>(physical) >> 21) & 0x1FF;
    auto directory_ptr = (reinterpret_cast<uintptr_t>(physical) >> 30) & 0x1FF;
    auto pml4 = (reinterpret_cast<uintptr_t>(physical) >> 39) & 0x1FF;

    //Find the entries
    pml4t_t pml4t = reinterpret_cast<pml4t_t>(0x70000);
    auto pdpt = reinterpret_cast<pdpt_t>(reinterpret_cast<uintptr_t>(pml4t[pml4]) & ~0xFFF);
    auto pdt = reinterpret_cast<pdt_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);
    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pdt[directory]) & ~0xFFF);

    //Identity map  the physical address
    pt[table] = reinterpret_cast<page_entry>(reinterpret_cast<uintptr_t>(physical) | 0x3);

    //TODO Check if pt[table] is already used and if so, return false

    return true;
}

bool paging::identity_map(void* physical, size_t pages){
    //The address must be page-aligned
    if(reinterpret_cast<uintptr_t>(physical) % PAGE_SIZE != 0){
        return false;
    }

    //TODO This should first check each page for the present bit

    for(size_t page = 0; page < pages; ++page){
        if(!identity_map(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(physical) + page * PAGE_SIZE))){
            return false;
        }
    }

    return true;
}
