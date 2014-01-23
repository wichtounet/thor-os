//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "malloc.hpp"
#include "console.hpp"
#include "physical_allocator.hpp"
#include "paging.hpp"
#include "e820.hpp"

namespace {

//Used to compile with malloc operations in the console
//can produce a lot of output
const bool DEBUG_MALLOC = false;
const bool TRACE_MALLOC = false;

size_t _used_memory;
size_t _allocated_memory;

struct malloc_footer_chunk;

class malloc_header_chunk {
private:
    size_t __size;
    malloc_header_chunk* __next;
    malloc_header_chunk* __prev;

public:
    size_t& size(){
        return __size;
    }

    constexpr malloc_header_chunk* next() const {
        return __next;
    }

    malloc_header_chunk*& next_ref(){
        return __next;
    }

    constexpr malloc_header_chunk* prev() const {
        return reinterpret_cast<malloc_header_chunk*>(reinterpret_cast<uintptr_t>(__prev) & (~0l - 1));
    }

    malloc_header_chunk*& prev_ref(){
        return __prev;
    }

    void set_free(){
        __prev = reinterpret_cast<malloc_header_chunk*>(reinterpret_cast<uintptr_t>(__prev) | 0x1);
    }

    void set_not_free(){
        __prev = reinterpret_cast<malloc_header_chunk*>(reinterpret_cast<uintptr_t>(__prev) & (~0l - 1));
    }

    bool is_free() const {
        return reinterpret_cast<uintptr_t>(__prev) & 0x1;
    }

    constexpr malloc_footer_chunk* footer() const {
        return reinterpret_cast<malloc_footer_chunk*>(
            reinterpret_cast<uintptr_t>(this) + __size + sizeof(malloc_header_chunk));
    }
};

struct malloc_footer_chunk {
    uint64_t __size;

    size_t& size(){
        return __size;
    }
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

constexpr size_t ALIGNMENT = 8;
constexpr size_t aligned_size(size_t bytes){
    return (bytes + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

static_assert(META_SIZE == aligned_size(META_SIZE), "The size of headers must be aligned");
static_assert(MIN_SPLIT == aligned_size(MIN_SPLIT), "The size of minimum split must be aligned");

fake_head head;
malloc_header_chunk* malloc_head = 0;

//All allocated memory is in [min_address, max_address[
uintptr_t min_address; //Address of the first block being allocated
uintptr_t max_address; //Address of the next block being allocated

uint64_t* allocate_block(uint64_t blocks){
    //Allocate the physical necessary memory
    auto physical_memory = physical_allocator::allocate(blocks);

    if(!physical_memory){
        return nullptr;
    }

    //Allocate the virtual necessary memory
    auto virtual_memory = virtual_allocator::allocate(blocks);

    if(!virtual_memory){
        return nullptr;
    }

    //Map the physical memory at the virtual address
    paging::map_pages(reinterpret_cast<void*>(virtual_memory), reinterpret_cast<void*>(physical_memory), blocks);

    if(min_address == 0){
        min_address = virtual_memory;
    } else {
        min_address = std::min(min_address, virtual_memory);
    }

    max_address = std::max(max_address, virtual_memory);

    _allocated_memory += blocks * BLOCK_SIZE;

    return reinterpret_cast<uint64_t*>(virtual_memory);
}

template<bool Debug>
void debug_malloc(const char* point = nullptr){
    if(Debug){
        if(point){
            k_print(point);
        }

        auto it = malloc_head;

        k_print(" next: ");
        do {
            k_printf("%u%h -> ", static_cast<size_t>(it->is_free()), reinterpret_cast<uint64_t>(it));
            it = it->next();
        } while(it != malloc_head);

        k_printf("%h\n", malloc_head);

        it = malloc_head;

        k_print("prev: ");
        do {
            k_printf("%h <- ", reinterpret_cast<uint64_t>(it));
            it = it->prev();
        } while(it != malloc_head);

        k_printf("%h\n", malloc_head);
    }
}

//Insert new_block after current in the free list and update
//all the necessary links
void insert_after(malloc_header_chunk* current, malloc_header_chunk* new_block){
    new_block->next_ref() = current->next();
    new_block->prev_ref() = current;
    new_block->set_free();

    current->next()->prev_ref() = new_block;
    current->next()->set_free();
    current->next_ref() = new_block;
}

//Remove the given block from the free list
//The node is directly marked as not free
void remove(malloc_header_chunk* current){
    current->prev()->next_ref() = current->next();
    current->next()->prev_ref() = current->prev();
    current->next()->set_free();

    //Make sure the node is clean
    current->prev_ref() = nullptr;
    current->next_ref() = nullptr;

    //No need to unmark the free bit here as we set current->next to zero
}

void init_head(){
    head.size = 0;
    head.next = nullptr;
    head.prev = nullptr;
    head.size_2 = 0;

    malloc_head = reinterpret_cast<malloc_header_chunk*>(&head);
    malloc_head->next_ref() = malloc_head;
    malloc_head->prev_ref() = malloc_head;
}

void expand_heap(malloc_header_chunk* current){
    //Allocate a new block of memory
    uint64_t* block = allocate_block(MIN_BLOCKS);

    //Transform it into a malloc chunk
    auto header = reinterpret_cast<malloc_header_chunk*>(block);

    //Update the sizes
    header->size() = MIN_BLOCKS * BLOCK_SIZE - META_SIZE;
    header->footer()->size() = header->size();

    //Insert the new block into the free list
    insert_after(current, header);
}

malloc_header_chunk* left_block(malloc_header_chunk* b){
    auto left_footer = reinterpret_cast<malloc_footer_chunk*>(
        reinterpret_cast<uintptr_t>(b) - sizeof(malloc_footer_chunk));

    if(reinterpret_cast<uintptr_t>(left_footer)>= min_address){
        auto left_size = left_footer->size();

        auto left_header = reinterpret_cast<malloc_header_chunk*>(
            reinterpret_cast<uintptr_t>(left_footer) - left_size - sizeof(malloc_header_chunk));

        if(reinterpret_cast<uintptr_t>(left_header) >= min_address){
            return left_header;
        }
    }

    return nullptr;
}

malloc_header_chunk* right_block(malloc_header_chunk* b){
    auto right_header = reinterpret_cast<malloc_header_chunk*>(
        reinterpret_cast<uintptr_t>(b) + META_SIZE + b->size());

    if(reinterpret_cast<uintptr_t>(right_header) < max_address){
        auto right_footer = right_header->footer();
        if(reinterpret_cast<uintptr_t>(right_footer) < max_address){
            return right_header;
        }
    }

    return nullptr;
}

malloc_header_chunk* coalesce(malloc_header_chunk* b){
    auto a = left_block(b);
    auto c = right_block(b);

    if(a && a->is_free()){
        auto new_size = a->size() + b->size() + META_SIZE;

        //Remove a from the free list
        remove(a);

        b = a;

        b->size() = new_size;
        b->footer()->size() = new_size;
    }

    if(c && c->is_free()){
        auto new_size = b->size() + c->size() + META_SIZE;

        //Remove c from the free list
        remove(c);

        b->size() = new_size;
        b->footer()->size() = new_size;
    }

    return b;
}

} //end of anonymous namespace

void malloc::init(){
    //Init the fake head
    init_head();

    //Allocate a first block
    expand_heap(malloc_head);
}

void* malloc::k_malloc(uint64_t bytes){
    auto current = malloc_head->next();

    //Try not to create too small blocks
    if(bytes < MIN_SPLIT){
        bytes = MIN_SPLIT;
    }

    //Make sure the addresses will be aligned on ALIGNMENT bytes
    bytes = aligned_size(bytes);

    while(true){
        if(current == malloc_head){
            //There are no blocks big enough to hold this request
            //So expand the heap
            expand_heap(current);
        } else if(current->size() >= bytes){
            //This block is big enough

            //Space necessary to hold a new block inside this one
            auto necessary_space = bytes + META_SIZE;

            //Is it worth splitting the block ?
            if(current->size() > necessary_space + MIN_SPLIT + META_SIZE){
                auto new_block_size = current->size() - necessary_space;

                //Set the new size of the current block
                current->size() = bytes;
                current->footer()->size() = bytes;

                //Create a new block inside the current one
                auto new_block = reinterpret_cast<malloc_header_chunk*>(
                    reinterpret_cast<uintptr_t>(current) + bytes + META_SIZE);

                //Update the size of the new block
                new_block->size() = new_block_size;
                new_block->footer()->size() = new_block->size();

                //Add the new block to the free list
                insert_after(current, new_block);

                //Remove the current block from the free list
                remove(current);

                debug_malloc<DEBUG_MALLOC>("after malloc split");

                break;
            } else {
                //Remove this node from the free list
                remove(current);

                debug_malloc<DEBUG_MALLOC>("after malloc no split");

                break;
            }
        }

        current = current->next();
    }

    _used_memory += current->size() + META_SIZE;

    auto b = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(current) + sizeof(malloc_header_chunk));

    if(TRACE_MALLOC){
        k_printf("m %u(%u) %h ", bytes, current->size(), reinterpret_cast<uint64_t>(b));
    }

    return b;
}

void malloc::k_free(void* block){
    auto free_header = reinterpret_cast<malloc_header_chunk*>(
        reinterpret_cast<uintptr_t>(block) - sizeof(malloc_header_chunk));

    if(free_header->is_free()){
        k_print_line("ERROR: free block getting freed");
    }

    if(TRACE_MALLOC){
        k_printf("f %u %h ", free_header->size(), reinterpret_cast<uint64_t>(block));
    }

    //Less memory is used
    _used_memory -= free_header->size() + META_SIZE;

    //Coalesce the block if possible
    free_header = coalesce(free_header);

    //Add the freed block in the free list
    insert_after(malloc_head, free_header);

    debug_malloc<DEBUG_MALLOC>("after free");
}

size_t malloc::allocated_memory(){
    return _allocated_memory;
}

size_t malloc::used_memory(){
    return _used_memory;
}

size_t malloc::free_memory(){
    size_t memory_free = 0;

    auto it = malloc_head;
    do {
        memory_free += it->size();

        it = it->next();
    } while(it != malloc_head);

    return memory_free;
}

void malloc::debug(){
    size_t memory_free = 0;
    size_t non_free_blocks = 0;
    size_t inconsistent = 0;

    auto it = malloc_head;

    k_printf("malloc overhead: %u\n", META_SIZE);
    k_print("Free blocks:");
    do {
        if(!it->is_free()){
            ++non_free_blocks;
            k_print("NF");
        }

        if(it->size() != it->footer()->size()){
            ++inconsistent;
        }

        k_printf("b(%u) ", it->size());
        memory_free += it->size();

        it = it->next();
    } while(it != malloc_head);

    k_print_line();
    k_printf("memory free in malloc chain: %m (%u)\n", memory_free, memory_free);
    k_printf("There are %u non free blocks in the free list\n", non_free_blocks);
    k_printf("There are %u inconsistent sized blocks in the free list\n", inconsistent);
}
