//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "kalloc.hpp"
#include "console.hpp"
#include "physical_allocator.hpp"
#include "paging.hpp"
#include "e820.hpp"
#include "int_lock.hpp"

#include "fs/sysfs.hpp"

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
    uint32_t _free;
    uint16_t left;
    uint16_t right;

public:
    size_t& size(){
        return __size;
    }

    bool has_left() const {
        return left;
    }

    bool has_right() const {
        return right;
    }

    void set_left(bool l){
        left = l;
    }

    void set_right(bool r){
        right = r;
    }

    malloc_header_chunk*& next(){
        return __next;
    }

    malloc_header_chunk*& prev(){
        return __prev;
    }

    void set_free(){
        _free = 1;
    }

    void set_not_free(){
        _free = 0;
    }

    bool is_free() const {
        return _free;
    }

    constexpr malloc_footer_chunk* footer() const {
        return reinterpret_cast<malloc_footer_chunk*>(
            reinterpret_cast<uintptr_t>(this) + __size + sizeof(malloc_header_chunk));
    }
};

struct malloc_footer_chunk {
private:
    uint64_t __size;

public:
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

constexpr size_t ALIGNMENT = 16;

constexpr bool aligned(size_t value){
    return (value & (ALIGNMENT - 1)) == 0;
}

constexpr const uint64_t META_SIZE = sizeof(malloc_header_chunk) + sizeof(malloc_footer_chunk);
constexpr const uint64_t BLOCK_SIZE = paging::PAGE_SIZE;
constexpr const uint64_t MIN_BLOCKS = 4;

constexpr const uint64_t MIN_SPLIT_BASE = ALIGNMENT == 8 ? 32 : 2 * ALIGNMENT;
constexpr const uint64_t MIN_SPLIT = MIN_SPLIT_BASE - sizeof(malloc_footer_chunk);

static_assert(aligned(sizeof(malloc_header_chunk)), "The header must be aligned");
static_assert(aligned(MIN_SPLIT + sizeof(malloc_footer_chunk)), "The size of minimum split must guarantee alignment");

fake_head head;
malloc_header_chunk* malloc_head = 0;

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
    paging::map_pages(virtual_memory, physical_memory, blocks);

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
            printf("%u%h -> ", static_cast<size_t>(it->is_free()), reinterpret_cast<uint64_t>(it));
            it = it->next();
        } while(it != malloc_head);

        printf("%h\n", malloc_head);

        it = malloc_head;

        k_print("prev: ");
        do {
            printf("%h <- ", reinterpret_cast<uint64_t>(it));
            it = it->prev();
        } while(it != malloc_head);

        printf("%h\n", malloc_head);
    }
}

//Insert new_block after current in the free list and update
//all the necessary links
void insert_after(malloc_header_chunk* current, malloc_header_chunk* new_block){
    //Link the new block to its surroundings
    new_block->next() = current->next();
    new_block->prev() = current;

    //Link surroundings to the new block
    current->next()->prev() = new_block;
    current->next() = new_block;

    new_block->set_free();
}

//Remove the given block from the free list
//The node is directly marked as not free
void remove(malloc_header_chunk* current){
    //Unlink the node
    current->prev()->next() = current->next();
    current->next()->prev() = current->prev();

    //Make sure the node is clean
    current->prev() = nullptr;
    current->next() = nullptr;

    //The node is not free anymore
    current->set_not_free();
}

void init_head(){
    head.size = 0;
    head.next = nullptr;
    head.prev = nullptr;
    head.size_2 = 0;

    malloc_head = reinterpret_cast<malloc_header_chunk*>(&head);
    malloc_head->next() = malloc_head;
    malloc_head->prev() = malloc_head;
    malloc_head->set_left(false);
    malloc_head->set_right(false);
}

void expand_heap(malloc_header_chunk* current, size_t bytes = 0){
    auto blocks = MIN_BLOCKS;

    if(bytes){
        auto necessary_blocks = ((bytes + META_SIZE) / BLOCK_SIZE) + 1;

        if(necessary_blocks > blocks){
            blocks = necessary_blocks;
        }
    }

    //Allocate a new block of memory
    uint64_t* block = allocate_block(blocks);

    //Transform it into a malloc chunk
    auto header = reinterpret_cast<malloc_header_chunk*>(block);

    //Update the sizes
    header->size() = blocks * BLOCK_SIZE - META_SIZE;
    header->footer()->size() = header->size();

    //When created a block has no left and right neighbour
    header->set_left(false);
    header->set_right(false);

    //Insert the new block into the free list
    insert_after(current, header);
}

malloc_header_chunk* left_block(malloc_header_chunk* b){
    if(b->has_left()){
        auto left_footer = reinterpret_cast<malloc_footer_chunk*>(
            reinterpret_cast<uintptr_t>(b) - sizeof(malloc_footer_chunk));

        auto left_size = left_footer->size();

        auto left_header = reinterpret_cast<malloc_header_chunk*>(
            reinterpret_cast<uintptr_t>(left_footer) - left_size - sizeof(malloc_header_chunk));

        return left_header;
    }

    return nullptr;
}

malloc_header_chunk* right_block(malloc_header_chunk* b){
    if(b->has_right()){
        auto right_header = reinterpret_cast<malloc_header_chunk*>(
            reinterpret_cast<uintptr_t>(b) + META_SIZE + b->size());

        return right_header;
    }

    return nullptr;
}

malloc_header_chunk* coalesce(malloc_header_chunk* b){
    auto a = left_block(b);
    auto c = right_block(b);

    if(a && a->is_free()){
        auto new_size = a->size() + b->size() + META_SIZE;

        auto block_left = a->has_left();
        auto block_right = b->has_right();

        //Remove a from the free list
        remove(a);

        b = a;

        b->size() = new_size;
        b->footer()->size() = new_size;

        b->set_left(block_left);
        b->set_right(block_right);
    }

    if(c && c->is_free()){
        auto new_size = b->size() + c->size() + META_SIZE;

        auto block_left = b->has_left();
        auto block_right = c->has_right();

        //Remove c from the free list
        remove(c);

        b->size() = new_size;
        b->footer()->size() = new_size;

        b->set_left(block_left);
        b->set_right(block_right);
    }

    return b;
}

std::string sysfs_free(){
    return std::to_string(kalloc::free_memory());
}

std::string sysfs_allocated(){
    return std::to_string(kalloc::allocated_memory());
}

std::string sysfs_used(){
    return std::to_string(kalloc::used_memory());
}

} //end of anonymous namespace

void kalloc::init(){
    //Init the fake head
    init_head();

    //Allocate a first block
    expand_heap(malloc_head);
}

void kalloc::finalize(){
    sysfs::set_dynamic_value("/sys/", "/memory/dynamic/free", &sysfs_free);
    sysfs::set_dynamic_value("/sys/", "/memory/dynamic/used", &sysfs_used);
    sysfs::set_dynamic_value("/sys/", "/memory/dynamic/allocated", &sysfs_allocated);
}

void* kalloc::k_malloc(uint64_t bytes){
    direct_int_lock lock;

    auto current = malloc_head->next();

    //Try not to create too small blocks
    if(bytes < MIN_SPLIT){
        bytes = MIN_SPLIT;
    }

    //Make sure that blocks will have the correct alignment at the end
    auto size_to_align = bytes + sizeof(malloc_footer_chunk);
    if(!aligned(size_to_align)){
        bytes = ((size_to_align / ALIGNMENT) + 1) * ALIGNMENT - sizeof(malloc_footer_chunk);
    }

    while(true){
        if(current == malloc_head){
            //There are no blocks big enough to hold this request
            //So expand the heap
            expand_heap(current, bytes);
        } else if(current->size() >= bytes){
            //This block is big enough

            //Space necessary to hold a new block inside this one
            auto necessary_space = bytes + META_SIZE;

            //Is it worth splitting the block ?
            if(current->size() > necessary_space + MIN_SPLIT + META_SIZE){
                auto new_block_size = current->size() - necessary_space;

                auto block_left = current->has_left();
                auto block_right = current->has_right();

                //Set the new size of the current block
                current->size() = bytes;
                current->footer()->size() = bytes;

                current->set_left(block_left);
                current->set_right(true);

                //Create a new block inside the current one
                auto new_block = reinterpret_cast<malloc_header_chunk*>(
                    reinterpret_cast<uintptr_t>(current) + bytes + META_SIZE);

                //Update the size of the new block
                new_block->size() = new_block_size;
                new_block->footer()->size() = new_block->size();

                new_block->set_left(true);
                new_block->set_right(block_right);

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

    //Address of the start of the block
    auto block_start = reinterpret_cast<uintptr_t>(current) + sizeof(malloc_header_chunk);

    if(TRACE_MALLOC){
        printf("m %u(%u) %h ", bytes, current->size(), block_start);
    }

    return reinterpret_cast<void*>(block_start);
}

void kalloc::k_free(void* block){
    direct_int_lock lock;

    auto free_header = reinterpret_cast<malloc_header_chunk*>(
        reinterpret_cast<uintptr_t>(block) - sizeof(malloc_header_chunk));

    if(free_header->is_free()){
        k_print_line("ERROR: free block getting freed");
    }

    if(TRACE_MALLOC){
        printf("f %u %h ", free_header->size(), reinterpret_cast<uint64_t>(block));
    }

    //Less memory is used
    _used_memory -= free_header->size() + META_SIZE;

    //Coalesce the block if possible
    free_header = coalesce(free_header);

    //Add the freed block in the free list
    insert_after(malloc_head, free_header);

    debug_malloc<DEBUG_MALLOC>("after free");
}

size_t kalloc::allocated_memory(){
    return _allocated_memory;
}

size_t kalloc::used_memory(){
    return _used_memory;
}

size_t kalloc::free_memory(){
    size_t memory_free = 0;

    auto it = malloc_head;
    do {
        memory_free += it->size();

        it = it->next();
    } while(it != malloc_head);

    return memory_free;
}

void kalloc::debug(){
    size_t memory_free = 0;
    size_t non_free_blocks = 0;
    size_t inconsistent = 0;

    auto it = malloc_head;

    printf("malloc overhead: %u\n", META_SIZE);
    k_print("Free blocks:");
    do {
        if(!it->is_free()){
            ++non_free_blocks;
            k_print("NF");
        }

        if(it->size() != it->footer()->size()){
            ++inconsistent;
        }

        printf("b(%u) ", it->size());
        memory_free += it->size();

        it = it->next();
    } while(it != malloc_head);

    k_print_line();
    printf("memory free in malloc chain: %m (%u)\n", memory_free, memory_free);
    printf("There are %u non free blocks in the free list\n", non_free_blocks);
    printf("There are %u inconsistent sized blocks in the free list\n", inconsistent);
}
