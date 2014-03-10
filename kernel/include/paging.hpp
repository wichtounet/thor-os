//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PAGING_H
#define PAGING_H

#include <types.hpp>
#include <literals.hpp>

#include "virtual_allocator.hpp"

//Forward declaration
namespace scheduler {
struct process_t;
}

namespace paging {

//The size of page in memory
constexpr const size_t PAGE_SIZE = 4096;

//The physical memory that a PML4T Entry can map
constexpr const size_t pml4e_allocations = 512_GiB;

//The physical memory that a PDPT Entry can map
constexpr const size_t pdpte_allocations = 1_GiB;

//The physical memory that a PD Entry can map
constexpr const size_t pde_allocations = 2_MiB;

//The physical memory that a PD Entry can map
constexpr const size_t pte_allocations = 4_KiB;

//Help function to compute the number of necessary entries to map the kernel
constexpr size_t entries(size_t entry_size){
    return entry_size >= virtual_allocator::kernel_virtual_size ?
        1 :
        virtual_allocator::kernel_virtual_size / entry_size + (virtual_allocator::kernel_virtual_size % entry_size == 0 ? 0 : 1);
}

//Compute the number of entries necessary to map the entire kernel virtual size
constexpr const auto pml4_entries = entries(pml4e_allocations);
constexpr const auto pdpt_entries = entries(pdpte_allocations);
constexpr const auto pd_entries = entries(pde_allocations);
constexpr const auto pt_entries = entries(pte_allocations);

//Virtual address where the paging structures are stored
constexpr const size_t virtual_paging_start = 0x101000;

//Compute the start address of each structure
constexpr const size_t virtual_pml4t_start = virtual_paging_start;
constexpr const size_t virtual_pdpt_start = virtual_pml4t_start + paging::PAGE_SIZE;
constexpr const size_t virtual_pd_start = virtual_pdpt_start + pml4_entries * paging::PAGE_SIZE;
constexpr const size_t virtual_pt_start = virtual_pd_start + pdpt_entries * paging::PAGE_SIZE;

//Compute the amount of physical memory pages needed for the paging tables
constexpr const size_t  physical_memory_pages = 1 + pml4_entries + pdpt_entries + pd_entries;

//Flags
constexpr const uint8_t PRESENT = 0x1;
constexpr const uint8_t WRITE = 0x2;
constexpr const uint8_t USER = 0x4;
constexpr const uint8_t WRITE_THROUGH = 0x8;
constexpr const uint8_t CACHE_DISABLED = 0x10;
constexpr const uint8_t ACCESSED= 0x20;

constexpr bool page_aligned(size_t addr){
    return !(addr & (paging::PAGE_SIZE - 1));
}

constexpr size_t page_align(size_t addr){
    return (addr / paging::PAGE_SIZE) * paging::PAGE_SIZE;
}

void init();
void finalize();

size_t physical_address(size_t virt);
bool page_present(size_t virt);
bool page_free_or_set(size_t virt, size_t physical);

bool identity_map(size_t virt, uint8_t flags = PRESENT | WRITE);
bool identity_map_pages(size_t virt, size_t pages, uint8_t flags = PRESENT | WRITE);

bool map(size_t virt, size_t physical, uint8_t flags = PRESENT | WRITE);
bool map_pages(size_t virt, size_t physical, size_t pages, uint8_t flags = PRESENT | WRITE);

bool unmap(size_t virt);
bool unmap_pages(size_t virt, size_t pages);

void map_kernel_inside_user(scheduler::process_t& process);
bool user_map(scheduler::process_t& process, size_t virt, size_t physical);
bool user_map_pages(scheduler::process_t& process, size_t virt, size_t physical, size_t pages);

size_t get_physical_pml4t();

} //end of namespace paging

#endif
