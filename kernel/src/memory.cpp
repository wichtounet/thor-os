//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "memory.hpp"
#include "console.hpp"
#include "paging.hpp"
#include "e820.hpp"

namespace {

//Used to compile with malloc operations in the console
//can produce a lot of output
const bool DEBUG_MALLOC = false;
const bool TRACE_MALLOC = false;

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

const e820::mmapentry* current_mmap_entry = nullptr;
uint64_t current_mmap_entry_position;

uint64_t* allocate_block(uint64_t blocks){
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

    if(bytes < MIN_SPLIT){
        bytes = MIN_SPLIT;
    }

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

            //Space necessary to hold a new block inside this one
            auto necessary_space = bytes + META_SIZE;

            //Is it worth splitting the block ?
            if(current->size > necessary_space + MIN_SPLIT + META_SIZE){
                auto new_block_size = current->size - necessary_space;

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

uint64_t used_memory(){
    return _used_memory;
}

uint64_t allocated_memory(){
    return _allocated_memory;
}

uint64_t free_memory(){
    return e820::available_memory() - _used_memory;
}

void memory_debug(){
    size_t memory_free = 0;

    auto it = malloc_head;

    k_printf("malloc overhead: %d\n", META_SIZE);
    k_print("Free blocks:");
    do {
        k_printf("b(%d) ", it->size);
        memory_free += it->size;
        it = it->next;
    } while(it != malloc_head);

    k_print_line();
    k_printf("memory free in malloc chain: %m (%d)", memory_free, memory_free);
}
