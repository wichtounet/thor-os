//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "memory.hpp"
#include "console.hpp"
#include "paging.hpp"

namespace {

//Used to compile with malloc operations in the console
//can produce a lot of output
const bool DEBUG_MALLOC = false;
const bool TRACE_MALLOC = false;

struct bios_mmap_entry {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint16_t type;
    uint16_t acpi;
    uint32_t damn_padding;
} __attribute__((packed));

uint64_t e820_failed = 0;
uint64_t entry_count = 0;
bios_mmap_entry* e820_address = 0;

mmapentry e820_mmap[32];

void mmap_query(uint64_t cmd, uint64_t* result){
    uint64_t tmp;

    __asm__ __volatile__ ("mov r8, %[port]; int 62; mov %[dst], rax"
        : [dst] "=a" (tmp)
        : [port] "dN" (cmd)
        : "cc", "memory", "r8");

    *result = tmp;
}

uint64_t _available_memory;
uint64_t _used_memory;
uint64_t _allocated_memory;

struct malloc_header_chunk {
    uint64_t size;
    malloc_header_chunk* next;
    malloc_header_chunk* prev;
};

struct malloc_footer_chunk {
    uint64_t size;
};

struct fake_head {
    uint64_t size;
    malloc_header_chunk* next;
    malloc_header_chunk* prev;
    uint64_t size_2;
};

constexpr const uint64_t META_SIZE = sizeof(malloc_header_chunk) + sizeof(malloc_footer_chunk);
constexpr const uint64_t MIN_SPLIT = 32;
constexpr const uint64_t BLOCK_SIZE = paging::PAGE_SIZE;
constexpr const uint64_t MIN_BLOCKS = 4;

fake_head head;
malloc_header_chunk* malloc_head = 0;

mmapentry* current_mmap_entry = nullptr;
uint64_t current_mmap_entry_position;

uint64_t* allocate_block(uint64_t blocks){
    if(!current_mmap_entry){
        for(uint64_t i = 0; i < entry_count; ++i){
            auto& entry = e820_mmap[i];

            if(entry.type == 1 && entry.base >= 0x100000 && entry.size >= 16384){
                current_mmap_entry = &entry;
                current_mmap_entry_position = entry.base;
                break;
            }
        }
    }

    if(!current_mmap_entry){
        return nullptr;
    }

    auto block = reinterpret_cast<uint64_t*>(current_mmap_entry_position);

    paging::identity_map(block, blocks);

    current_mmap_entry_position += blocks * BLOCK_SIZE;

    _allocated_memory += blocks * BLOCK_SIZE;

    return block;
}

template<bool Debug>
void debug_malloc(const char* point = nullptr){
    if(Debug){
        if(point){
            k_print_line(point);
        }

        auto it = malloc_head;

        k_print("next: ");
        do {
            k_printf("%h -> ", reinterpret_cast<uint64_t>(it));
            it = it->next;
        } while(it != malloc_head);

        k_printf("%h\n", malloc_head);

        it = malloc_head;

        k_print("prev: ");
        do {
            k_printf("%h <- ", reinterpret_cast<uint64_t>(it));
            it = it->prev;
        } while(it != malloc_head);

        k_printf("%h\n", malloc_head);
    }
}

} //end of anonymous namespace

void init_memory_manager(){
    //Init the fake head
    head.size = 0;
    head.next = nullptr;
    head.prev = nullptr;
    head.size_2 = 0;

    malloc_head = reinterpret_cast<malloc_header_chunk*>(&head);

    auto block = allocate_block(MIN_BLOCKS);
    auto header = reinterpret_cast<malloc_header_chunk*>(block);

    header->size = MIN_BLOCKS * BLOCK_SIZE - META_SIZE;
    header->next = malloc_head;
    header->prev = malloc_head;

    auto footer = reinterpret_cast<malloc_footer_chunk*>(
        reinterpret_cast<uintptr_t>(block) + header->size + sizeof(malloc_header_chunk));
    footer->size = header->size;

    malloc_head->next = header;
    malloc_head->prev = header;
}

void* k_malloc(uint64_t bytes){
    auto current = malloc_head->next;

    while(true){
        if(current == malloc_head){
            //There are no blocks big enough to hold this request

            uint64_t* block = allocate_block(MIN_BLOCKS);
            auto header = reinterpret_cast<malloc_header_chunk*>(block);
            header->size = MIN_BLOCKS * BLOCK_SIZE - META_SIZE;

            header->next = current->next;
            header->prev = current;

            current->next->prev = header;
            current->next = header;

            auto footer = reinterpret_cast<malloc_footer_chunk*>(
                reinterpret_cast<uintptr_t>(block) + header->size + sizeof(malloc_header_chunk));
            footer->size = header->size;
        } else if(current->size >= bytes){
            //This block is big enough

            //Is it worth splitting the block ?
            if(current->size > bytes + META_SIZE + MIN_SPLIT){
                auto new_block_size = current->size - bytes - META_SIZE;

                //Set the new size;
                current->size = bytes;

                auto footer = reinterpret_cast<malloc_footer_chunk*>(
                    reinterpret_cast<uintptr_t>(current) + bytes + sizeof(malloc_header_chunk));
                footer->size = bytes;

                auto new_block = reinterpret_cast<malloc_header_chunk*>(
                    reinterpret_cast<uintptr_t>(current) + bytes + META_SIZE);

                new_block->size = new_block_size;
                new_block->next = current->next;
                new_block->prev = current->prev;
                current->prev->next = new_block;
                current->next->prev = new_block;

                auto new_footer = reinterpret_cast<malloc_footer_chunk*>(
                    reinterpret_cast<uintptr_t>(new_block) + new_block_size + sizeof(malloc_header_chunk));
                new_footer->size = new_block_size;

                debug_malloc<DEBUG_MALLOC>("after malloc split");

                break;
            } else {
                //Remove this node from the free list
                current->prev->next = current->next;
                current->next->prev = current->prev;

                debug_malloc<DEBUG_MALLOC>("after malloc no split");

                break;
            }
        }

        current = current->next;
    }

    _used_memory += current->size + META_SIZE;

    //Make sure the node is clean
    current->prev = nullptr;
    current->next = nullptr;

    auto b = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(current) + sizeof(malloc_header_chunk));

    if(TRACE_MALLOC){
        k_printf("m %d(%d) %h ", bytes, current->size, reinterpret_cast<uint64_t>(b));
    }

    return b;
}

void k_free(void* block){
    auto free_header = reinterpret_cast<malloc_header_chunk*>(
        reinterpret_cast<uintptr_t>(block) - sizeof(malloc_header_chunk));

    if(TRACE_MALLOC){
        k_printf("f %d %h ",free_header->size, reinterpret_cast<uint64_t>(block));
    }

    _used_memory -= free_header->size + META_SIZE;

    auto header = malloc_head;

    free_header->prev = header;
    free_header->next = header->next;

    header->next->prev = free_header;
    header->next = free_header;

    debug_malloc<DEBUG_MALLOC>("after free");
}

void load_memory_map(){
    mmap_query(0, &e820_failed);
    mmap_query(1, &entry_count);
    mmap_query(2, reinterpret_cast<uint64_t*>(&e820_address));

    if(!e820_failed && e820_address){
        for(uint64_t i = 0; i < entry_count; ++i){
            auto& bios_entry = e820_address[i];
            auto& os_entry = e820_mmap[i];

            uint64_t base = bios_entry.base_low + (static_cast<uint64_t>(bios_entry.base_high) << 32);
            uint64_t length = bios_entry.length_low + (static_cast<uint64_t>(bios_entry.length_high) << 32);

            os_entry.base = base;
            os_entry.size = length;
            os_entry.type = bios_entry.type;

            if(os_entry.base == 0 && os_entry.type == 1){
                os_entry.type = 7;
            }

            if(os_entry.type == 1){
                _available_memory += os_entry.size;
            }
        }
    }
}

uint64_t mmap_entry_count(){
    return entry_count;
}

bool mmap_failed(){
    return e820_failed;
}

const mmapentry& mmap_entry(uint64_t i){
    return e820_mmap[i];
}

const char* str_e820_type(uint64_t type){
    switch(type){
        case 1:
            return "Free";
        case 2:
            return "Reserved";
        case 3:
        case 4:
            return "ACPI";
        case 5:
            return "Unusable";
        case 6:
            return "Disabled";
        case 7:
            return "Kernel";
        default:
            return "Unknown";
    }
}

uint64_t available_memory(){
    return _available_memory;
}

uint64_t used_memory(){
    return _used_memory;
}

uint64_t allocated_memory(){
    return _allocated_memory;
}

uint64_t free_memory(){
    return _available_memory - _used_memory;
}
