//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "paging.hpp"
#include "physical_allocator.hpp"

#include "stl/types.hpp"
#include "stl/algorithms.hpp"

namespace {

typedef uint64_t* page_entry;
typedef page_entry* pt_t;
typedef pt_t* pd_t;
typedef pd_t* pdpt_t;
typedef pdpt_t* pml4t_t;

//Memory from 0x70000 can be used for pages
uintptr_t last_page = 0x73000;

uintptr_t init_new_page(){
    auto new_page = last_page + paging::PAGE_SIZE;
    auto it = reinterpret_cast<size_t*>(new_page);

    std::fill(it, it + paging::PAGE_SIZE / sizeof(size_t), 0);

    last_page = new_page;

    return new_page;
}

inline void flush_tlb(void* page){
    asm volatile("invlpg [%0]" :: "r" (reinterpret_cast<uintptr_t>(page)) : "memory");
}

} //end of anonymous namespace

void paging::init(){
    //PD[0] already points to a valid PT (0x73000)
    auto pt = reinterpret_cast<pt_t>(0x73000);

    auto pd = reinterpret_cast<pd_t>(0x72000);

    auto physical = physical_allocator::early_allocate(1);

    pt[256] = reinterpret_cast<page_entry>(physical | PRESENT | WRITE | USER);
    flush_tlb(reinterpret_cast<void*>(0x100000));

    auto it = reinterpret_cast<size_t*>(0x100000);
    std::fill(it, it + paging::PAGE_SIZE / sizeof(size_t), 0);

    pd[1] = reinterpret_cast<pt_t>(physical | PRESENT | WRITE | USER);

    //Use PD[1] as pt
    pt = reinterpret_cast<pt_t>(0x100000);

    for(size_t pd_index = 2; pd_index < 512; ++pd_index){
        //1. Allocate space for the new Page Table
        physical = physical_allocator::early_allocate(1);

        //2. Compute logical address
        uint64_t logical = 0x200000 + (pd_index - 2) * paging::PAGE_SIZE;

        //2. Map it using the valid entries in the first PT
        pt[pd_index - 2] = reinterpret_cast<page_entry>(physical | PRESENT | WRITE | USER);
        flush_tlb(reinterpret_cast<void*>(logical));

        //3. Clear the new PT
        it = reinterpret_cast<size_t*>(logical);
        std::fill(it, it + paging::PAGE_SIZE / sizeof(size_t), 0);

        //4. Set it in PD
        pd[pd_index] = reinterpret_cast<pt_t>(physical | PRESENT | WRITE | USER);
    }
}

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
    auto pd = reinterpret_cast<pd_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);
    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pd[directory]) & ~0xFFF);
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

    auto pd = reinterpret_cast<pd_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);
    if(!(reinterpret_cast<uintptr_t>(pd[directory]) & PRESENT)){
        return false;
    }

    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pd[directory]) & ~0xFFF);
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

bool paging::map(void* virt, void* physical, uint8_t flags){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //Find the correct indexes inside the paging table for the virtual address
    auto table = (reinterpret_cast<uintptr_t>(virt) >> 12) & 0x1FF;
    auto directory = (reinterpret_cast<uintptr_t>(virt) >> 21) & 0x1FF;
    auto directory_ptr = (reinterpret_cast<uintptr_t>(virt) >> 30) & 0x1FF;
    auto pml4 = (reinterpret_cast<uintptr_t>(virt) >> 39) & 0x1FF;

    pml4t_t pml4t = reinterpret_cast<pml4t_t>(0x70000);

    //Init new page if necessary
    if(!(reinterpret_cast<uintptr_t>(pml4t[pml4]) & PRESENT)){
        pml4t[pml4] = reinterpret_cast<pdpt_t>(init_new_page() | USER | WRITE | PRESENT);
    }

    auto pdpt = reinterpret_cast<pdpt_t>(reinterpret_cast<uintptr_t>(pml4t[pml4]) & ~0xFFF);

    //Init new page if necessary
    if(!(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & PRESENT)){
        pdpt[directory_ptr] = reinterpret_cast<pd_t>(init_new_page() | USER | WRITE | PRESENT);
    }

    auto pd = reinterpret_cast<pd_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);

    //Init new page if necessary
    if(!(reinterpret_cast<uintptr_t>(pd[directory]) & PRESENT)){
        pd[directory] = reinterpret_cast<pt_t>(init_new_page() | USER | WRITE | PRESENT);
    }

    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pd[directory]) & ~0xFFF);

    //Check if the page is already present
    if(reinterpret_cast<uintptr_t>(pt[table]) & PRESENT){
        //If the page is already set to the correct value, return true
        //If the page is set to another value, return false
        return reinterpret_cast<uintptr_t>(pt[table]) == (reinterpret_cast<uintptr_t>(physical) | flags);
    }

    //Map to the physical address
    pt[table] = reinterpret_cast<page_entry>(reinterpret_cast<uintptr_t>(physical) | flags);

    //Flush TLB
    flush_tlb(virt);

    return true;
}

bool paging::map_pages(void* virt, void* physical, size_t pages, uint8_t flags){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //To avoid mapping only a subset of the pages
    //check if one of the page is already mapped to another value
    for(size_t page = 0; page < pages; ++page){
        auto virt_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(virt) + page * PAGE_SIZE);
        auto phys_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(physical) + page * PAGE_SIZE);

        if(!page_free_or_set(virt_addr, phys_addr)){
            return false;
        }
    }

    //Identity map each page
    for(size_t page = 0; page < pages; ++page){
        auto virt_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(virt) + page * PAGE_SIZE);
        auto phys_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(physical) + page * PAGE_SIZE);

        if(!map(virt_addr, phys_addr, flags)){
            return false;
        }
    }

    return true;
}

bool paging::unmap(void* virt){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //Find the correct indexes inside the paging table for the virtual address
    auto table = (reinterpret_cast<uintptr_t>(virt) >> 12) & 0x1FF;
    auto directory = (reinterpret_cast<uintptr_t>(virt) >> 21) & 0x1FF;
    auto directory_ptr = (reinterpret_cast<uintptr_t>(virt) >> 30) & 0x1FF;
    auto pml4 = (reinterpret_cast<uintptr_t>(virt) >> 39) & 0x1FF;

    pml4t_t pml4t = reinterpret_cast<pml4t_t>(0x70000);

    //If not present, returns directly
    if(!(reinterpret_cast<uintptr_t>(pml4t[pml4]) & PRESENT)){
        return true;
    }

    auto pdpt = reinterpret_cast<pdpt_t>(reinterpret_cast<uintptr_t>(pml4t[pml4]) & ~0xFFF);

    //If not present, returns directly
    if(!(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & PRESENT)){
        return true;
    }

    auto pd = reinterpret_cast<pd_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);

    //If not present, returns directly
    if(!(reinterpret_cast<uintptr_t>(pd[directory]) & PRESENT)){
        return true;
    }

    auto pt = reinterpret_cast<pt_t>(reinterpret_cast<uintptr_t>(pd[directory]) & ~0xFFF);

    //Unmap the virtual address
    pt[table] = 0x0;

    //Flush TLB
    flush_tlb(virt);

    return true;
}

bool paging::unmap_pages(void* virt, size_t pages){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //Unmap each page
    for(size_t page = 0; page < pages; ++page){
        auto virt_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(virt) + page * PAGE_SIZE);

        if(!unmap(virt_addr)){
            return false;
        }
    }

    return true;
}

bool paging::identity_map(void* virt, uint8_t flags){
    return map(virt, virt, flags);
}

bool paging::identity_map_pages(void* virt, size_t pages, uint8_t flags){
    return map_pages(virt, virt, pages, flags);
}
