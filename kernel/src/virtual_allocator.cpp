//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <array.hpp>

#include "virtual_allocator.hpp"
#include "paging.hpp"
#include "buddy_allocator.hpp"
#include "assert.hpp"
#include "logging.hpp"

#include "fs/sysfs.hpp"

//For problems during boot
#include "kernel.hpp"
#include "console.hpp"

namespace {

size_t virtual_start;
size_t first_virtual_address;
size_t last_virtual_address;
size_t managed_space;

constexpr const size_t unit = paging::PAGE_SIZE;

size_t allocated_pages = first_virtual_address / paging::PAGE_SIZE;

constexpr size_t array_size(int block){
    return (virtual_allocator::kernel_virtual_size / (block * unit) + 1) / (sizeof(uint64_t) * 8) + 1;
}

std::array<uint64_t, array_size(1)> data_bitmap_1;
std::array<uint64_t, array_size(2)> data_bitmap_2;
std::array<uint64_t, array_size(4)> data_bitmap_4;
std::array<uint64_t, array_size(8)> data_bitmap_8;
std::array<uint64_t, array_size(16)> data_bitmap_16;
std::array<uint64_t, array_size(32)> data_bitmap_32;
std::array<uint64_t, array_size(64)> data_bitmap_64;
std::array<uint64_t, array_size(128)> data_bitmap_128;

typedef buddy_allocator<8, unit> buddy_type;
buddy_type allocator;

std::string sysfs_free(){
    return std::to_string(virtual_allocator::free());
}

std::string sysfs_available(){
    return std::to_string(virtual_allocator::available());
}

std::string sysfs_allocated(){
    return std::to_string(virtual_allocator::allocated());
}

} //end of anonymous namespace

void virtual_allocator::init(){
    // The first addressable virtual address is just after the paging structures
    virtual_start = paging::virtual_paging_start + (paging::physical_memory_pages * paging::PAGE_SIZE);

    // Take the next first aligned 1MiB virtual address
    first_virtual_address = virtual_start % 0x100000 == 0 ? virtual_start : (virtual_start / 0x100000 + 1) * 0x100000;
    last_virtual_address = virtual_allocator::kernel_virtual_size;
    managed_space = last_virtual_address - first_virtual_address;

    allocator.set_memory_range(first_virtual_address, last_virtual_address);

    //Give room to the bitmaps
    allocator.init<0>(array_size(1), data_bitmap_1.data());
    allocator.init<1>(array_size(2), data_bitmap_2.data());
    allocator.init<2>(array_size(4), data_bitmap_4.data());
    allocator.init<3>(array_size(8), data_bitmap_8.data());
    allocator.init<4>(array_size(16), data_bitmap_16.data());
    allocator.init<5>(array_size(32), data_bitmap_32.data());
    allocator.init<6>(array_size(64), data_bitmap_64.data());
    allocator.init<7>(array_size(128), data_bitmap_128.data());

    allocator.init();
}

void virtual_allocator::finalize(){
    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/memory/virtual/available"), &sysfs_available);
    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/memory/virtual/free"), &sysfs_free);
    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/memory/virtual/allocated"), &sysfs_allocated);
}

size_t virtual_allocator::allocate(size_t pages){
    thor_assert(pages < free() / paging::PAGE_SIZE, "Not enough virtual memory");

    allocated_pages += buddy_type::level_size(pages);

    auto virt = allocator.allocate(pages);

    if(!virt){
        logging::logf(logging::log_level::ERROR, "valloc: Unable to allocate %u pages\n", size_t(pages));
    }

    return virt;
}

void virtual_allocator::free(size_t address, size_t pages){
    allocated_pages -= buddy_type::level_size(pages);

    allocator.free(address, pages);
}

size_t virtual_allocator::available(){
    return kernel_virtual_size;
}

size_t virtual_allocator::allocated(){
    return allocated_pages * paging::PAGE_SIZE;
}

size_t virtual_allocator::free(){
    return available() - allocated();
}
