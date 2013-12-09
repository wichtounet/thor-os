//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "paging.hpp"
#include "types.hpp"
#include "utils.hpp"

#include "console.hpp"

namespace {

typedef uint64_t* page_entry;
typedef page_entry* pt_t;
typedef pt_t* pdt_t;
typedef pdt_t* pdpt_t;
typedef pdpt_t* pml4t_t;

constexpr const int PRESENT = 0x1;
constexpr const int WRITEABLE = 0x2;
constexpr const int USER = 0x4;

//Memory from 0x70000 can be used for pages
uintptr_t last_page = 0x73000;

uintptr_t init_new_page(){
    auto new_page = last_page + paging::PAGE_SIZE;
    auto it = reinterpret_cast<size_t*>(new_page);

    std::fill(it, it + paging::PAGE_SIZE / sizeof(size_t), 0);

    last_page = new_page;

    return new_page;
}

} //end of anonymous namespace

//TODO Update to support offsets at the end of virt
//TODO Improve to support a status
void* paging::physical_address(void* virt){
    if(!page_present(virt)){
        return nullptr;
    }

    //Find the correct indexes inside the paging table for the physical address
    auto table = (reinterpret_cast<uintptr_t>(virt) >> 12) & 0x1FF;
    auto directory = (reinterpret_cast<uintptr_t>(virt) >> 21) & 0x1FF;
    auto directory_ptr = (reinterpret_cast<uintptr_t>(virt) >> 30) & 0x1FF;
    auto pml4 = (reinterpret_cast<uintptr_t>(virt) >> 39) & 0x1FF;

    pml4t_t pml4t = reinterpret_cast<pml4t_t>(0x70000);

    auto pdpt = reinterpret_cast<pdpt_t>(reinterpret_cast<uintptr_t>(pml4t[pml4]) & ~0xFFF);
    auto pdt = reinterpret_cast<pdt_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);
    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pdt[directory]) & ~0xFFF);
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pt[table]) & ~0xFFF);
}

bool paging::page_present(void* virt){
    //Find the correct indexes inside the paging table for the physical address
    auto table = (reinterpret_cast<uintptr_t>(virt) >> 12) & 0x1FF;
    auto directory = (reinterpret_cast<uintptr_t>(virt) >> 21) & 0x1FF;
    auto directory_ptr = (reinterpret_cast<uintptr_t>(virt) >> 30) & 0x1FF;
    auto pml4 = (reinterpret_cast<uintptr_t>(virt) >> 39) & 0x1FF;

    pml4t_t pml4t = reinterpret_cast<pml4t_t>(0x70000);
    if(!(reinterpret_cast<uintptr_t>(pml4t[pml4]) & PRESENT)){
        return false;
    }

    auto pdpt = reinterpret_cast<pdpt_t>(reinterpret_cast<uintptr_t>(pml4t[pml4]) & ~0xFFF);
    if(!(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & PRESENT)){
        return false;
    }

    auto pdt = reinterpret_cast<pdt_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);
    if(!(reinterpret_cast<uintptr_t>(pdt[directory]) & PRESENT)){
        return false;
    }

    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pdt[directory]) & ~0xFFF);
    return reinterpret_cast<uintptr_t>(pt[table]) & PRESENT;
}

bool paging::page_free_or_set(void* virt, void* physical){
    if(!page_present(virt)){
        return true;
    }

    if(physical_address(virt) == physical){
        return true;
    }

    return false;
}

bool paging::identity_map(void* virt){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //Find the correct indexes inside the paging table for the physical address
    auto table = (reinterpret_cast<uintptr_t>(virt) >> 12) & 0x1FF;
    auto directory = (reinterpret_cast<uintptr_t>(virt) >> 21) & 0x1FF;
    auto directory_ptr = (reinterpret_cast<uintptr_t>(virt) >> 30) & 0x1FF;
    auto pml4 = (reinterpret_cast<uintptr_t>(virt) >> 39) & 0x1FF;

    pml4t_t pml4t = reinterpret_cast<pml4t_t>(0x70000);

    //Init new page if necessary
    if(!(reinterpret_cast<uintptr_t>(pml4t[pml4]) & PRESENT)){
        pml4t[pml4] = reinterpret_cast<pdpt_t>(init_new_page() | (PRESENT | WRITEABLE));
    }

    auto pdpt = reinterpret_cast<pdpt_t>(reinterpret_cast<uintptr_t>(pml4t[pml4]) & ~0xFFF);

    //Init new page if necessary
    if(!(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & PRESENT)){
        pdpt[directory_ptr] = reinterpret_cast<pdt_t>(init_new_page() | (PRESENT | WRITEABLE));
    }

    auto pdt = reinterpret_cast<pdt_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);

    //Init new page if necessary
    if(!(reinterpret_cast<uintptr_t>(pdt[directory]) & PRESENT)){
        pdt[directory] = reinterpret_cast<pt_t>(init_new_page() | (PRESENT | WRITEABLE));
    }

    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pdt[directory]) & ~0xFFF);

    //Check if the page is already present
    if(reinterpret_cast<uintptr_t>(pt[table]) & PRESENT){
        //If the page is already set to the correct value, return true
        //If the page is set to another value, return false
        return reinterpret_cast<uintptr_t>(pt[table]) == (reinterpret_cast<uintptr_t>(virt) | (PRESENT | WRITEABLE));
    }

    //Identity map  the physical address
    pt[table] = reinterpret_cast<page_entry>(reinterpret_cast<uintptr_t>(virt) | (PRESENT | WRITEABLE));

    return true;
}

bool paging::identity_map(void* virt, size_t pages){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //To avoid mapping only a subset of the pages
    //check if one of the page is already mapped to another value
    for(size_t page = 0; page < pages; ++page){
        auto addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(virt) + page * PAGE_SIZE);
        if(!page_free_or_set(addr, addr)){
            return false;
        }
    }

    //Identity map each page
    for(size_t page = 0; page < pages; ++page){
        if(!identity_map(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(virt) + page * PAGE_SIZE))){
            return false;
        }
    }

    return true;
}
