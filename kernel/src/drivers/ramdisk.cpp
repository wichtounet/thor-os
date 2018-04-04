//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/errors.hpp>

#include "drivers/ramdisk.hpp"

#include "disks.hpp"
#include "paging.hpp"
#include "logging.hpp"

namespace {

constexpr const size_t MAX_RAMDISK = 3;

size_t current = 0;
ramdisk::disk_descriptor ramdisks[MAX_RAMDISK];

} //end of anonymous namespace

ramdisk::disk_descriptor* ramdisk::make_disk(uint64_t max_size){
    if(current == MAX_RAMDISK){
        return nullptr;
    }

    auto pages = paging::pages(max_size);

    ramdisks[current].id = current;
    ramdisks[current].max_size = max_size;
    ramdisks[current].pages = pages;
    ramdisks[current].allocated = new char*[pages];

    for(size_t i = 0; i < pages; ++i){
        ramdisks[current].allocated[i] = nullptr;
    }

    logging::logf(logging::log_level::TRACE, "ramdisk: Created ramdisk %u of size %m with %u pages\n", current, max_size, pages);

    ++current;
    return &ramdisks[current - 1];
}

size_t ramdisk::ramdisk_driver::read(void* data, char* destination, size_t count, size_t offset, size_t& read){
    read = 0;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);

    if(offset + count >= disk->max_size){
        logging::logf(logging::log_level::ERROR, "ramdisk: Tried to read too far\n");
        return std::ERROR_INVALID_OFFSET;
    }

    while(read != count){
        auto page = offset / paging::PAGE_SIZE;
        auto page_offset = offset % paging::PAGE_SIZE;

        auto to_read = std::min(paging::PAGE_SIZE - page_offset, count - read);

        // If the page is not allocated, we simply consider it full of zero
        if(!disk->allocated[page]){
            std::fill_n(destination + read, to_read, 0);
        } else {
            std::copy_n(disk->allocated[page] + page_offset, to_read, destination + read);
        }

        read += to_read;
        offset += to_read;
    }

    return 0;
}

size_t ramdisk::ramdisk_driver::write(void* data, const char* source, size_t count, size_t offset, size_t& written){
    written = 0;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);

    if(offset + count >= disk->max_size){
        logging::logf(logging::log_level::ERROR, "ramdisk: Tried to write too far\n");
        return std::ERROR_INVALID_OFFSET;
    }

    while(written != count){
        auto page = offset / paging::PAGE_SIZE;
        auto page_offset = offset % paging::PAGE_SIZE;

        if(!disk->allocated[page]){
            logging::logf(logging::log_level::TRACE, "ramdisk: Disk %u Allocated page %u \n", disk->id, page);

            disk->allocated[page] = new char[paging::PAGE_SIZE];
            std::fill_n(disk->allocated[page], paging::PAGE_SIZE, 0);
        }

        uint64_t to_write = std::min(paging::PAGE_SIZE - page_offset, count - written);
        std::copy_n(source, to_write, disk->allocated[page] + page_offset);
        written += to_write;

        offset += to_write;
    }

    return 0;
}

size_t ramdisk::ramdisk_driver::clear(void* data, size_t count, size_t offset, size_t& written){
    written = 0;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);

    if(offset + count >= disk->max_size){
        logging::logf(logging::log_level::ERROR, "ramdisk: Tried to write too far\n");
        return std::ERROR_INVALID_OFFSET;
    }

    while(written != count){
        auto page = offset / paging::PAGE_SIZE;
        auto page_offset = offset % paging::PAGE_SIZE;

        uint64_t to_write = std::min(paging::PAGE_SIZE - page_offset, count - written);

        // No need to allocate page, they are filled with zeros the
        // first time they are allocated
        if(disk->allocated[page]){
            std::fill_n(disk->allocated[page] + page_offset, to_write, 0);
        }

        written += to_write;
        offset += to_write;
    }

    return 0;
}

size_t ramdisk::ramdisk_driver::size(void* data){
    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);
    return disk->max_size;
}
