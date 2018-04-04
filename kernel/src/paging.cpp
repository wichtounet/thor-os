//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <types.hpp>
#include <algorithms.hpp>
#include <math.hpp>

#include "paging.hpp"
#include "kernel.hpp"
#include "physical_allocator.hpp"
#include "print.hpp"
#include "assert.hpp"
#include "process.hpp"
#include "physical_pointer.hpp"
#include "kernel_utils.hpp"
#include "logging.hpp"
#include "early_memory.hpp"

#include "fs/sysfs.hpp"

size_t paging::virtual_paging_start;
size_t paging::virtual_early_page;

size_t paging::virtual_pml4t_start;
size_t paging::virtual_pdpt_start;
size_t paging::virtual_pd_start;
size_t paging::virtual_pt_start;

namespace {

typedef uint64_t* page_entry;
typedef page_entry* pt_t;
typedef pt_t* pd_t;
typedef pd_t* pdpt_t;
typedef pdpt_t* pml4t_t;

//The physical offsets of the structures
size_t physical_pml4t_start;
size_t physical_pdpt_start;
size_t physical_pd_start;
size_t physical_pt_start;

constexpr size_t pml4_entry(size_t virt){
    return (virt >> 39) & 0x1FF;
}

constexpr size_t pdpt_entry(size_t virt){
    return (virt >> 30) & 0x1FF;
}

constexpr size_t pd_entry(size_t virt){
    return (virt >> 21) & 0x1FF;
}

constexpr size_t pt_entry(size_t virt){
    return (virt >> 12) & 0x1FF;
}

pml4t_t find_pml4t(){
    return reinterpret_cast<pml4t_t>(paging::virtual_pml4t_start);
}

pdpt_t find_pdpt(pml4t_t pml4t, size_t pml4e){
    auto physical_pdpt = reinterpret_cast<uintptr_t>(pml4t[pml4e]) & ~0xFFF;
    auto physical_offset = physical_pdpt - physical_pdpt_start;
    auto virtual_pdpt = paging::virtual_pdpt_start + physical_offset;
    return reinterpret_cast<pdpt_t>(virtual_pdpt);
}

pd_t find_pd(pdpt_t pdpt, size_t pdpte){
    auto physical_pd = reinterpret_cast<uintptr_t>(pdpt[pdpte]) & ~0xFFF;
    auto physical_offset = physical_pd - physical_pd_start;
    auto virtual_pd = paging::virtual_pd_start + physical_offset;
    return reinterpret_cast<pd_t>(virtual_pd);
}

pt_t find_pt(pd_t pd, size_t pde){
    auto physical_pt = reinterpret_cast<uintptr_t>(pd[pde]) & ~0xFFF;
    auto physical_offset = physical_pt - physical_pt_start;
    auto virtual_pt = paging::virtual_pt_start + physical_offset;
    return reinterpret_cast<pt_t>(virtual_pt);
}

inline void flush_tlb(size_t page){
    asm volatile("invlpg [%0]" :: "r" (page) : "memory");
}

size_t early_map_page(size_t physical){
    thor_assert(paging::virtual_early_page < 0x100000, "Invalid early page");

    //PD[0] already points to a valid PT (0x73000)
    auto pt = reinterpret_cast<pt_t>(0x73000);

    // The first 256 entries are used for identity mapping the first 1Mib
    // The page 256 can safely be used

    pt[paging::virtual_early_page / 0x1000] = reinterpret_cast<page_entry>(physical | paging::PRESENT | paging::WRITE);
    flush_tlb(paging::virtual_early_page);

    return paging::virtual_early_page;
}

size_t early_map_page_clear(size_t physical){
    auto virt = early_map_page(physical);

    //Clear the new allocated block of memory
    auto it = reinterpret_cast<size_t*>(virt);
    std::fill_n(it, paging::PAGE_SIZE / sizeof(size_t), 0);

    return virt;
}

} //end of anonymous namespace

void paging::early_init(){
    logging::logf(logging::log_level::TRACE, "Kernel occupies %u MiB from %h\n", uint64_t(early::kernel_mib()), uint64_t(early::kernel_address));

    // This page is used for mapping the physical address of the paging structure before
    // paging is completely initialized. It won't be used after final paging is setup
    paging::virtual_early_page = 0x20000;

    // The paging structures lies in memory just after the kernel code itself
    paging::virtual_paging_start = early::kernel_address + 0x100000 * early::kernel_mib();

    //Compute the start address of each structure
    paging::virtual_pml4t_start = virtual_paging_start;
    paging::virtual_pdpt_start = virtual_pml4t_start + paging::PAGE_SIZE;
    paging::virtual_pd_start = virtual_pdpt_start + pml4_entries * paging::PAGE_SIZE;
    paging::virtual_pt_start = virtual_pd_start + pdpt_entries * paging::PAGE_SIZE;

    logging::logf(logging::log_level::TRACE, "paging: early_init (virtual_paging_start:%h)\n", virtual_paging_start);
}

void paging::init(){
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

    logging::logf(logging::log_level::TRACE, "paging: init (physical_memory:%h, physical_pages:%u)\n", physical_memory, physical_memory_pages);
    logging::logf(logging::log_level::TRACE, "paging: PML4T entries:%u phys:%h virt:%h\n", pml4_entries, physical_pml4t_start, virtual_pml4t_start);
    logging::logf(logging::log_level::TRACE, "paging: PDPT entries:%u phys:%h virt:%h\n", pdpt_entries, physical_pdpt_start, virtual_pdpt_start);
    logging::logf(logging::log_level::TRACE, "paging: PD entries:%u phys:%h virt:%h\n", pd_entries, physical_pd_start, virtual_pd_start);
    logging::logf(logging::log_level::TRACE, "paging: PT entries:%u phys:%h virt:%h\n", pt_entries, physical_pt_start, virtual_pt_start);

    auto high_flags = PRESENT | WRITE | USER;

    //1. Prepare PML4T
    auto virt = early_map_page_clear(physical_pml4t_start);
    for(size_t i = 0; i < pml4_entries; ++i){
        (reinterpret_cast<pml4t_t>(virt))[i] = reinterpret_cast<pdpt_t>((physical_pdpt_start + i * PAGE_SIZE) | high_flags);
    }

    //2. Prepare each PDPT
    for(size_t i = 0; i < pml4_entries; ++i){
        virt = early_map_page_clear(physical_pdpt_start + i * PAGE_SIZE);

        for(size_t j = 0; j + i * 512 < pdpt_entries; ++j){
            auto r = j + i * 512;

            (reinterpret_cast<pdpt_t>(virt))[j] = reinterpret_cast<pd_t>((physical_pd_start + r * PAGE_SIZE) | high_flags);
        }
    }

    //3. Prepare each PD
    for(size_t i = 0; i < pdpt_entries; ++i){
        virt = early_map_page_clear(physical_pd_start + i * PAGE_SIZE);

        for(size_t j = 0; j + i * 512 < pd_entries; ++j){
            auto r = j + i * 512;

            (reinterpret_cast<pd_t>(virt))[j] = reinterpret_cast<pt_t>((physical_pt_start + r * PAGE_SIZE) | high_flags);
        }
    }

    //4. Prepare each PT
    for(size_t i = 0; i < pd_entries; ++i){
        early_map_page_clear(physical_pt_start + i * PAGE_SIZE);
    }

    //5. Identity map the first MiB and the kernel code

    auto current_pt_phys = physical_pt_start;
    virt = early_map_page_clear(current_pt_phys);
    auto page_table_ptr = reinterpret_cast<uint64_t*>(virt);
    auto phys = PRESENT | WRITE;
    for(size_t i = 0; i < 256 + 256 * early::kernel_mib(); ++i){
        *page_table_ptr = phys;

        phys += paging::PAGE_SIZE;

        ++page_table_ptr;

        if(i > 0 && i % 511 == 0){
            current_pt_phys += paging::PAGE_SIZE;
            virt = early_map_page_clear(current_pt_phys);
            page_table_ptr = reinterpret_cast<uint64_t*>(virt);
        }
    }

    //6. Map all the paging structures

    auto phys_page = physical_memory;
    auto virt_page = virtual_paging_start;

    size_t current_pt_index = 0;
    uintptr_t current_virt = 0;
    for(size_t i = 0; i < physical_memory_pages; ++i){
        auto pml4e = pml4_entry(virt_page);
        auto pdpte = pdpt_entry(virt_page);
        auto pde = pd_entry(virt_page);
        auto pte = pt_entry(virt_page);

        auto pt_index = pde * 512 + pdpte * 512 * 512 + pml4e * 512 * 512 * 512;
        auto physical = physical_pt_start + (pt_index / 512) * paging::PAGE_SIZE;

        if(pt_index != current_pt_index || current_virt == 0){
            current_virt = early_map_page(physical);
        }

        (reinterpret_cast<pt_t>(current_virt))[pte] = reinterpret_cast<page_entry>(phys_page | PRESENT | WRITE);

        current_pt_index = pt_index;

        virt_page += paging::PAGE_SIZE;
        phys_page += paging::PAGE_SIZE;
    }

    //7. Use the new structure as the new paging structure (in CR3)

    logging::logf(logging::log_level::TRACE, "paging: Set %h (physical) as new CR3\n", physical_pml4t_start);

    asm volatile("mov rax, %0; mov cr3, rax" : : "m"(physical_pml4t_start) : "memory", "rax");

    //8. Perform some basic tests

    logging::logf(logging::log_level::TRACE, "paging: basic tests\n");

    if(!page_present(virtual_pml4t_start)){
        logging::logf(logging::log_level::ERROR, "paging: PML4T is not correctly mapped\n");
        suspend_boot();
    }

    if(physical_address(virtual_pml4t_start) != physical_pml4t_start){
        logging::logf(logging::log_level::ERROR, "paging: PML4T is not correctly mapped\n");
        suspend_boot();
    }

    // The first MiB must be identity mapped
    if(physical_address(0x6666) != 0x6666){
        logging::logf(logging::log_level::ERROR, "paging: Invalid identity mapping\n");
        suspend_boot();
    }

    logging::logf(logging::log_level::TRACE, "paging: basic tests finished\n");
}

void paging::finalize(){
    sysfs::set_constant_value(sysfs::get_sys_path(), path("/paging/page_size"), std::to_string(paging::PAGE_SIZE));
    sysfs::set_constant_value(sysfs::get_sys_path(), path("/paging/pdpt"), std::to_string(paging::pml4_entries));
    sysfs::set_constant_value(sysfs::get_sys_path(), path("/paging/pd"), std::to_string(paging::pdpt_entries));
    sysfs::set_constant_value(sysfs::get_sys_path(), path("/paging/pt"), std::to_string(paging::pd_entries));
    sysfs::set_constant_value(sysfs::get_sys_path(), path("/paging/physical_size"), std::to_string(paging::physical_memory_pages * paging::PAGE_SIZE));
}

size_t paging::pages(size_t size){
    return std::ceil_divide(size, PAGE_SIZE);
}

//TODO Improve to support a status
size_t paging::physical_address(size_t virt){
    if(!page_present(virt)){
        return 0;
    }

    // Offset inside the page
    auto offset = virt & uint64_t((1 << 13) - 1);

    //Find the correct indexes inside the paging table for the physical address
    auto pml4e = pml4_entry(virt);
    auto pdpte = pdpt_entry(virt);
    auto pde = pd_entry(virt);
    auto pte = pt_entry(virt);

    auto pml4t = find_pml4t();
    auto pdpt = find_pdpt(pml4t, pml4e);
    auto pd = find_pd(pdpt, pdpte);
    auto pt = find_pt(pd, pde);

    return offset + (reinterpret_cast<uintptr_t>(pt[pte]) & ~0xFFF);
}

bool paging::page_present(size_t virt){
    //Find the correct indexes inside the paging table for the physical address
    auto pml4e = pml4_entry(virt);
    auto pdpte = pdpt_entry(virt);
    auto pde = pd_entry(virt);
    auto pte = pt_entry(virt);

    auto pml4t = find_pml4t();
    if(!(reinterpret_cast<uintptr_t>(pml4t[pml4e]) & PRESENT)){
        return false;
    }

    auto pdpt = find_pdpt(pml4t, pml4e);
    if(!(reinterpret_cast<uintptr_t>(pdpt[pdpte]) & PRESENT)){
        return false;
    }

    auto pd = find_pd(pdpt, pdpte);
    if(!(reinterpret_cast<uintptr_t>(pd[pde]) & PRESENT)){
        return false;
    }

    auto pt = find_pt(pd, pde);
    return reinterpret_cast<uintptr_t>(pt[pte]) & PRESENT;
}

bool paging::page_free_or_set(size_t virt, size_t physical){
    if(!page_present(virt)){
        return true;
    }

    if(physical_address(virt) == physical){
        return true;
    }

    return false;
}

bool paging::map(size_t virt, size_t physical, uint8_t flags){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //Find the correct indexes inside the paging table for the virtual address
    auto pml4e = pml4_entry(virt);
    auto pdpte = pdpt_entry(virt);
    auto pde = pd_entry(virt);
    auto pte = pt_entry(virt);

    auto pml4t = find_pml4t();
    thor_assert(reinterpret_cast<uintptr_t>(pml4t[pml4e]) & PRESENT, "A PML4T entry is not PRESENT");

    auto pdpt = find_pdpt(pml4t, pml4e);
    thor_assert(reinterpret_cast<uintptr_t>(pdpt[pdpte]) & PRESENT, "A PDPT entry is not PRESENT");

    auto pd = find_pd(pdpt, pdpte);
    thor_assert(reinterpret_cast<uintptr_t>(pd[pde]) & PRESENT, "A PD entry is not PRESENT");

    auto pt = find_pt(pd, pde);

    //Check if the page is already present
    if(reinterpret_cast<uintptr_t>(pt[pte]) & PRESENT){
        //If the page is already set to the correct value, return true
        //If the page is set to another value, return false
        return reinterpret_cast<uintptr_t>(pt[pte]) == (physical | flags);
    }

    //Map to the physical address
    pt[pte] = reinterpret_cast<page_entry>(physical | flags);

    //Flush TLB
    flush_tlb(virt);

    return true;
}

bool paging::map_pages(size_t virt, size_t physical, size_t pages, uint8_t flags){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //To avoid mapping only a subset of the pages
    //check if one of the page is already mapped to another value
    for(size_t page = 0; page < pages; ++page){
        auto virt_addr = virt + page * PAGE_SIZE;
        auto phys_addr = physical + page * PAGE_SIZE;

        if(!page_free_or_set(virt_addr, phys_addr)){
            return false;
        }
    }

    //Map each page
    for(size_t page = 0; page < pages; ++page){
        auto virt_addr = virt + page * PAGE_SIZE;
        auto phys_addr = physical + page * PAGE_SIZE;

        if(!map(virt_addr, phys_addr, flags)){
            return false;
        }
    }

    return true;
}

bool paging::unmap(size_t virt){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //Find the correct indexes inside the paging table for the virtual address
    auto pml4e = pml4_entry(virt);
    auto pdpte = pdpt_entry(virt);
    auto pde = pd_entry(virt);
    auto pte = pt_entry(virt);

    auto pml4t = find_pml4t();

    //If not present, returns directly
    if(!(reinterpret_cast<uintptr_t>(pml4t[pml4e]) & PRESENT)){
        return true;
    }

    auto pdpt = find_pdpt(pml4t, pml4e);

    //If not present, returns directly
    if(!(reinterpret_cast<uintptr_t>(pdpt[pdpte]) & PRESENT)){
        return true;
    }

    auto pd = find_pd(pdpt, pdpte);

    //If not present, returns directly
    if(!(reinterpret_cast<uintptr_t>(pd[pde]) & PRESENT)){
        return true;
    }

    auto pt = find_pt(pd, pde);

    //Unmap the virtual address
    pt[pte] = 0x0;

    //Flush TLB
    flush_tlb(virt);

    return true;
}

bool paging::unmap_pages(size_t virt, size_t pages){
    //The address must be page-aligned
    if(!page_aligned(virt)){
        return false;
    }

    //Unmap each page
    for(size_t page = 0; page < pages; ++page){
        auto virt_addr = virt + page * PAGE_SIZE;

        if(!unmap(virt_addr)){
            return false;
        }
    }

    return true;
}

bool paging::identity_map(size_t virt, uint8_t flags){
    return map(virt, virt, flags);
}

bool paging::identity_map_pages(size_t virt, size_t pages, uint8_t flags){
    return map_pages(virt, virt, pages, flags);
}
void paging::map_kernel_inside_user(scheduler::process_t& process){
    physical_pointer cr3_ptr(process.physical_cr3, 1);

    //As we are ensuring that the first PML4T entries are reserved to the
    //kernel, it is enough to link these ones to the kernel ones

    auto pml4t = cr3_ptr.as<pml4t_t>();
    for(size_t i = 0; i < pml4_entries; ++i){
        pml4t[i] = reinterpret_cast<pdpt_t>((physical_pdpt_start + i * PAGE_SIZE) | USER | PRESENT);
    }
}

void clear_physical_page(size_t physical){
    physical_pointer ptr(physical, 1);

    auto it = ptr.as_ptr<uint64_t>();
    std::fill_n(it, paging::PAGE_SIZE / sizeof(uint64_t), 0);
}

//TODO It is highly inefficient to remap CR3 each time
bool paging::user_map(scheduler::process_t& process, size_t virt, size_t physical){
    physical_pointer cr3_ptr(process.physical_cr3, 1);

    if(!cr3_ptr){
        return false;
    }

    //Find the correct indexes inside the paging table for the virtual address
    auto pml4e = pml4_entry(virt);
    auto pdpte = pdpt_entry(virt);
    auto pde = pd_entry(virt);
    auto pte = pt_entry(virt);

    auto pml4t = cr3_ptr.as<pml4t_t>();
    if(!(reinterpret_cast<uintptr_t>(pml4t[pml4e]) & PRESENT)){
        auto physical_pdpt = physical_allocator::allocate(1);

        pml4t[pml4e] = reinterpret_cast<pdpt_t>(physical_pdpt | WRITE | USER | PRESENT);

        clear_physical_page(physical_pdpt);

        process.paging_size += paging::PAGE_SIZE;
        process.segments.emplace_back(physical_pdpt, 1UL);
    }

    auto physical_pdpt = reinterpret_cast<uintptr_t>(pml4t[pml4e]) & ~0xFFF;
    physical_pointer pdpt_ptr(physical_pdpt, 1);

    if(!pdpt_ptr){
        return false;
    }

    auto pdpt = pdpt_ptr.as<pdpt_t>();
    if(!(reinterpret_cast<uintptr_t>(pdpt[pdpte]) & PRESENT)){
        auto physical_pd = physical_allocator::allocate(1);

        pdpt[pdpte] = reinterpret_cast<pd_t>(physical_pd | WRITE | USER | PRESENT);

        clear_physical_page(physical_pd);

        process.paging_size += paging::PAGE_SIZE;
        process.segments.emplace_back(physical_pdpt, 1UL);
    }

    auto physical_pd = reinterpret_cast<uintptr_t>(pdpt[pdpte]) & ~0xFFF;
    physical_pointer pd_ptr(physical_pd, 1);

    if(!pd_ptr){
        return false;
    }

    auto pd = pd_ptr.as<pd_t>();
    if(!(reinterpret_cast<uintptr_t>(pd[pde]) & PRESENT)){
        auto physical_pt = physical_allocator::allocate(1);

        pd[pde] = reinterpret_cast<pt_t>(physical_pt | WRITE | USER | PRESENT);

        clear_physical_page(physical_pt);

        process.paging_size += paging::PAGE_SIZE;
        process.segments.emplace_back(physical_pdpt, 1UL);
    }

    auto physical_pt = reinterpret_cast<uintptr_t>(pd[pde]) & ~0xFFF;
    physical_pointer pt_ptr(physical_pt, 1);

    if(!pt_ptr){
        return false;
    }

    auto pt = pt_ptr.as<pt_t>();

    //Map to the physical address
    pt[pte] = reinterpret_cast<page_entry>(physical | WRITE | USER | PRESENT);

    return true;
}

bool paging::user_map_pages(scheduler::process_t& process, size_t virt, size_t physical, size_t pages){
    //Map each page
    for(size_t page = 0; page < pages; ++page){
        auto virt_addr = virt + page * PAGE_SIZE;
        auto phys_addr = physical + page * PAGE_SIZE;

        if(!user_map(process, virt_addr, phys_addr)){
            return false;
        }
    }

    return true;
}

size_t paging::get_physical_pml4t(){
    return physical_pml4t_start;
}
