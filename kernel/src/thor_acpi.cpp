//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
 * This file contains the OS specific layer
 */

#include "acpica.hpp"

#include "kalloc.hpp"
#include "console.hpp"
#include "scheduler.hpp"
#include "virtual_allocator.hpp"
#include "paging.hpp"

extern "C" {

void* AcpiOsAllocate(ACPI_SIZE size){
    return kalloc::k_malloc(size);
}

void AcpiOsFree(void* p){
    kalloc::k_free(p);
}

void AcpiOsPrintf(const char* format, ...){
    va_list va;
    va_start(va, format);

    printf(format, va);

    va_end(va);
}

void AcpiOsVprintf(const char* format, va_list va){
    printf(format, va);
}

ACPI_THREAD_ID AcpiOsGetThreadId(void){
    return scheduler::get_pid();
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(){
    ACPI_PHYSICAL_ADDRESS  root_pointer;
    root_pointer = 0;
    AcpiFindRootPointer(&root_pointer);
    return root_pointer;
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS phys, ACPI_SIZE length){
    size_t pages = (length + paging::PAGE_SIZE - 1) & ~(paging::PAGE_SIZE - 1);

    auto virt = virtual_allocator::allocate(pages);
    auto phys_aligned = phys - (phys & ~(paging::PAGE_SIZE - 1));

    paging::map_pages(virt, phys_aligned, pages);

    return reinterpret_cast<void*>(virt + (phys & ~(paging::PAGE_SIZE - 1)));
}

void AcpiOsUnmapMemory(void* virt_aligned_raw, ACPI_SIZE length){
    size_t pages = (length + paging::PAGE_SIZE - 1) & ~(paging::PAGE_SIZE - 1);

    auto virt_aligned = reinterpret_cast<size_t>(virt_aligned_raw);
    auto virt = virt_aligned - (virt_aligned & ~(paging::PAGE_SIZE - 1));

    paging::unmap_pages(virt, pages);
    virtual_allocator::free(virt, pages);
}

} //end of extern "C"
