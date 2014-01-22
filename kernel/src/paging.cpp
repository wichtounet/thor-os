//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "paging.hpp"
#include "kernel.hpp"
#include "physical_allocator.hpp"
#include "console.hpp"
#include "assert.hpp"

#include "stl/types.hpp"
#include "stl/algorithms.hpp"
#include "stl/literals.hpp"

namespace {

typedef uint64_t* page_entry;
typedef page_entry* pt_t;
typedef pt_t* pd_t;
typedef pd_t* pdpt_t;
typedef pdpt_t* pml4t_t;

//The virtual size allocated to the kernel
constexpr const size_t kernel_virtual_size = 1_GiB;

//The physical memory that a PML4T Entry can map
constexpr const size_t pml4e_allocations = 512_GiB;

//The physical memory that a PDPT Entry can map
constexpr const size_t pdpte_allocations = 1_GiB;

//The physical memory that a PD Entry can map
constexpr const size_t pde_allocations = 2_MiB;

//The physical memory that a PD Entry can map
constexpr const size_t pte_allocations = 4_KiB;

//Virtual address where the paging structures are stored
constexpr const size_t virtual_paging_start = 0x100000;

inline void flush_tlb(void* page){
    asm volatile("invlpg [%0]" :: "r" (reinterpret_cast<uintptr_t>(page)) : "memory");
}

constexpr size_t entries(size_t entry_size){
    return entry_size >= kernel_virtual_size ?
        1 :
        kernel_virtual_size / entry_size + (kernel_virtual_size % entry_size == 0 ? 0 : 1);
}

//Compute the number of entries necessary to map the entire kernel virtual size
constexpr const auto pml4_entries = entries(pml4e_allocations);
constexpr const auto pdpt_entries = entries(pdpte_allocations);
constexpr const auto pd_entries = entries(pde_allocations);
constexpr const auto pt_entries = entries(pte_allocations);

//Compute the start address of each structures
constexpr const size_t virtual_pml4t_start = virtual_paging_start;
constexpr const size_t virtual_pdpt_start = virtual_pml4t_start + paging::PAGE_SIZE;
constexpr const size_t virtual_pd_start = virtual_pdpt_start + pml4_entries * paging::PAGE_SIZE;
constexpr const size_t virtual_pt_start = virtual_pd_start + pdpt_entries * paging::PAGE_SIZE;

//The physical offsets of the structures
size_t physical_pml4t_start;
size_t physical_pdpt_start;
size_t physical_pd_start;
size_t physical_pt_start;

size_t early_map_page(size_t physical){
    //PD[0] already points to a valid PT (0x73000)
    auto pt = reinterpret_cast<pt_t>(0x73000);

    pt[256] = reinterpret_cast<page_entry>(physical | paging::PRESENT | paging::WRITE);
    flush_tlb(reinterpret_cast<void*>(0x100000));

    //Clear the new allocated block of memory
    auto it = reinterpret_cast<size_t*>(0x100000);
    std::fill(it, it + paging::PAGE_SIZE / sizeof(size_t), 0);

    return 0x100000;
}

} //end of anonymous namespace

void paging::init(){
    //Compute the amount of physical memory pages needed for the paging tables
    auto physical_memory_pages = 1 + pml4_entries + pdpt_entries + pd_entries;

    //Get some physical memory
    auto physical_memory = physical_allocator::early_allocate(physical_memory_pages);

    if(!physical_memory){
        k_print_line("Impossible to allocate enough physical memory for paging tables");

        suspend_boot();
    }

    //Compute the physical offsets of the paging tables
    physical_pml4t_start = physical_memory;
    physical_pdpt_start = physical_pml4t_start + paging::PAGE_SIZE;
    physical_pd_start = physical_pdpt_start + pml4_entries * paging::PAGE_SIZE;
    physical_pt_start = physical_pd_start + pdpt_entries * paging::PAGE_SIZE;

    auto high_flags = PRESENT | WRITE | USER;

    //1. Prepare PML4T
    auto virt = early_map_page(physical_pml4t_start);
    for(size_t i = 0; i < pml4_entries; ++i){
        (reinterpret_cast<pml4t_t>(virt))[i] = reinterpret_cast<pdpt_t>((physical_pdpt_start + i * PAGE_SIZE) | high_flags);
    }

    //2. Prepare each PDPT
    for(size_t i = 0; i < pml4_entries; ++i){
        virt = early_map_page(physical_pdpt_start + i * PAGE_SIZE);

        for(size_t j = 0; j + i * 512 < pdpt_entries; ++j){
            auto r = j + i * 512;

            (reinterpret_cast<pdpt_t>(virt))[r] = reinterpret_cast<pd_t>((physical_pd_start + i * PAGE_SIZE) | high_flags);
        }
    }

    //3. Prepare each PD
    for(size_t i = 0; i < pdpt_entries; ++i){
        virt = early_map_page(physical_pd_start + i * PAGE_SIZE);

        for(size_t j = 0; j + i * 512 < pd_entries; ++j){
            auto r = j + i * 512;

            (reinterpret_cast<pd_t>(virt))[r] = reinterpret_cast<pt_t>((physical_pt_start + i * PAGE_SIZE) | high_flags);
        }
    }

    //4. Prepare each PT
    for(size_t i = 0; i < pd_entries; ++i){
        early_map_page(physical_pt_start + i * PAGE_SIZE);
    }

    //5. Identity map the first MiB

    virt = early_map_page(physical_pt_start);
    auto page_table_ptr = reinterpret_cast<uint64_t*>(virt);
    auto phys = PRESENT | WRITE;
    for(size_t i = 0; i < 256; ++i){
        *page_table_ptr = phys;

        phys += paging::PAGE_SIZE;

        ++page_table_ptr;
    }

    //6. Use the new structure as the new paging structure (in CR3)

    asm volatile("mov rax, %0; mov cr3, rax" : : "m"(physical_pml4t_start) : "memory", "rax");
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

    thor_assert(reinterpret_cast<uintptr_t>(pml4t[pml4]) & PRESENT, "A PML4T entry is not PRESENT");

    auto pdpt = reinterpret_cast<pdpt_t>(reinterpret_cast<uintptr_t>(pml4t[pml4]) & ~0xFFF);

    thor_assert(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & PRESENT, "A PDPT entry is not PRESENT");

    auto pd = reinterpret_cast<pd_t>(reinterpret_cast<uintptr_t>(pdpt[directory_ptr]) & ~0xFFF);

    thor_assert(reinterpret_cast<uintptr_t>(pd[directory_ptr]) & PRESENT, "A PD entry is not PRESENT");

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
