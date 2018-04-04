//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "mmap.hpp"
#include "paging.hpp"
#include "virtual_allocator.hpp"
#include "logging.hpp"

void* mmap_phys(size_t phys, size_t size){
    auto offset = phys % paging::PAGE_SIZE;
    auto aligned_phys = phys - offset;

    auto real_length = offset + size;
    auto pages = paging::pages(real_length);

    // Allocate pages of virtual memory

    auto virt = virtual_allocator::allocate(pages);

    if(!virt){
        logging::logf(logging::log_level::ERROR, "mmap: Unable to allocate %u virtual pages\n", size_t(pages));
        return nullptr;
    }

    // Map to the physical address

    if(!paging::map_pages(virt, aligned_phys, pages)){
        logging::logf(logging::log_level::ERROR, "mmap: Unable to map %u pages %h->%h\n", size_t(pages), size_t(virt), size_t(aligned_phys));
        return nullptr;
    }

    return reinterpret_cast<void*>(virt);
}

bool munmap_phys(void* virt_ptr, size_t size){
    auto virt = reinterpret_cast<size_t>(virt_ptr);

    auto offset = virt % paging::PAGE_SIZE;
    auto aligned_virt = virt - offset;

    auto real_length = offset + size;
    auto pages = paging::pages(real_length);

    // Release the virtual memory

    virtual_allocator::free(aligned_virt, pages);

    // Unmap the memory

    if(!paging::unmap_pages(aligned_virt, pages)){
        logging::logf(logging::log_level::ERROR, "munmap: Unable to unmap %u pages %h\n", size_t(pages), size_t(aligned_virt));
        return false;
    }

    return true;
}
