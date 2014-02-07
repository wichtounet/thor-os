//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "physical_allocator.hpp"
#include "e820.hpp"
#include "paging.hpp"
#include "buddy_allocator.hpp"
#include "assert.hpp"

//For problems during boot
#include "kernel.hpp"
#include "console.hpp"

namespace {

constexpr const size_t unit = paging::PAGE_SIZE;

const e820::mmapentry* current_mmap_entry = 0;
uintptr_t current_mmap_entry_position = 0;

size_t allocated_memory = 0;

typedef buddy_allocator<8, unit> buddy_type;
buddy_type allocator;

size_t first_physical_address;
size_t last_physical_address;

size_t array_size(size_t managed_space, size_t block){
    return (managed_space / (block * unit) + 1) / (sizeof(uint64_t) * 8) + 1;
}

uint64_t* create_array(size_t managed_space, size_t block){
    auto size = array_size(managed_space, block) * sizeof(uint64_t);
    auto physical_address = current_mmap_entry_position;

    allocated_memory += size;
    current_mmap_entry_position += size;

    auto pages = size / paging::PAGE_SIZE + 1;
    auto virtual_address = virtual_allocator::allocate(pages);

    thor_assert(paging::map_pages(virtual_address, physical_address, pages), "Impossible to map pages for the physical allocator");

    return reinterpret_cast<uint64_t*>(virtual_address);
}

} //End of anonymous namespace

void physical_allocator::early_init(){
    e820::finalize_memory_detection();

    if(e820::mmap_failed()){
        k_print_line("e820 failed, no way to allocate memory");
        suspend_boot();
    }
}

size_t physical_allocator::early_allocate(size_t blocks){
    if(!current_mmap_entry){
        for(uint64_t i = 0; i < e820::mmap_entry_count(); ++i){
            auto& entry = e820::mmap_entry(i);

            if(entry.type == 1 && entry.base >= 0x100000 && entry.size >= 16384){
                current_mmap_entry = &entry;
                current_mmap_entry_position = entry.base;

                break;
            }
        }
    }

    if(!current_mmap_entry){
        return 0;
    }

    allocated_memory += blocks * paging::PAGE_SIZE;

    auto address = current_mmap_entry_position;

    current_mmap_entry_position += blocks * paging::PAGE_SIZE;

    //TODO If we are at the end of the block, we gonna have a problem

    return address;
}

void physical_allocator::init(){
    auto size = current_mmap_entry->size;
    auto managed_space = size - (current_mmap_entry_position -  current_mmap_entry->base);

    auto data_bitmap_1 = create_array(managed_space, 1);
    auto data_bitmap_2 = create_array(managed_space, 2);
    auto data_bitmap_4 = create_array(managed_space, 4);
    auto data_bitmap_8 = create_array(managed_space, 8);
    auto data_bitmap_16 = create_array(managed_space, 16);
    auto data_bitmap_32 = create_array(managed_space, 32);
    auto data_bitmap_64 = create_array(managed_space, 64);
    auto data_bitmap_128 = create_array(managed_space, 128);

    first_physical_address = current_mmap_entry_position;
    last_physical_address = current_mmap_entry->base + current_mmap_entry->size;

    allocator.set_memory_range(first_physical_address, last_physical_address);

    allocator.init<0>(array_size(managed_space, 1), data_bitmap_1);
    allocator.init<1>(array_size(managed_space, 2), data_bitmap_2);
    allocator.init<2>(array_size(managed_space, 4), data_bitmap_4);
    allocator.init<3>(array_size(managed_space, 8), data_bitmap_8);
    allocator.init<4>(array_size(managed_space, 16), data_bitmap_16);
    allocator.init<5>(array_size(managed_space, 32), data_bitmap_32);
    allocator.init<6>(array_size(managed_space, 64), data_bitmap_64);
    allocator.init<7>(array_size(managed_space, 128), data_bitmap_128);

    //TODO The current system uses more memory than necessary,
    //because it also tries to index memory that is used for the
    //buddy system.
    //A two pass computation of the arrays size could probably
    //solve this
}

size_t physical_allocator::allocate(size_t blocks){
    thor_assert(blocks < free() / paging::PAGE_SIZE, "Not enough virtual memory");

    allocated_memory += buddy_type::level_size(blocks) * unit;

    return allocator.allocate(blocks);
}

size_t physical_allocator::available(){
    return e820::available_memory();
}

size_t physical_allocator::allocated(){
    return allocated_memory;
}

size_t physical_allocator::free(){
    return available() - allocated();
}
