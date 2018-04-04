//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/malloc.hpp"

#define likely(x)    __builtin_expect (!!(x), 1)
#define unlikely(x)  __builtin_expect (!!(x), 0)

namespace {

bool init = false;

size_t _used = 0;
size_t _allocated = 0;

struct malloc_header_chunk {
    size_t size;
    malloc_header_chunk* next;
    malloc_header_chunk* prev;
    size_t padding;
};

struct fake_head {
    uint64_t size;
    malloc_header_chunk* next;
    malloc_header_chunk* prev;
};

constexpr size_t ALIGNMENT = 16;

constexpr bool aligned(size_t value){
    return (value & (ALIGNMENT - 1)) == 0;
}

constexpr const uint64_t META_SIZE = sizeof(malloc_header_chunk);
constexpr const uint64_t BLOCK_SIZE = 4096;
constexpr const uint64_t MIN_BLOCKS = 4;

constexpr const uint64_t MIN_SPLIT = ALIGNMENT == 8 ? 32 : 2 * ALIGNMENT;

static_assert(aligned(sizeof(malloc_header_chunk)), "The header must be aligned");
static_assert(aligned(MIN_SPLIT), "The size of minimum split must guarantee alignment");

fake_head head;
malloc_header_chunk* malloc_head = 0;

//Insert new_block after current in the free list and update
//all the necessary links
void insert_after(malloc_header_chunk* current, malloc_header_chunk* new_block){
    //Link the new block to its surroundings
    new_block->next = current->next;
    new_block->prev = current;

    //Link surroundings to the new block
    current->next->prev = new_block;
    current->next = new_block;
}

//Remove the given block from the free list
//The node is directly marked as not free
void remove(malloc_header_chunk* current){
    //Unlink the node
    current->prev->next = current->next;
    current->next->prev = current->prev;

    //Make sure the node is clean
    current->prev = nullptr;
    current->next = nullptr;
}

bool expand_heap(malloc_header_chunk* current, size_t bytes = 0){
    auto blocks = MIN_BLOCKS;

    if(unlikely(bytes)){
        auto necessary_blocks = ((bytes + META_SIZE) / BLOCK_SIZE) + 1;

        if(necessary_blocks > blocks){
            blocks = necessary_blocks;
        }
    }

    //Allocate a new block of memory
    auto old_end = tlib::brk_end();
    auto brk_end = tlib::sbrk(blocks * BLOCK_SIZE);

    if(brk_end == old_end){
        return false;
    }

    auto real_blocks = (brk_end - old_end) / BLOCK_SIZE;

    _allocated += real_blocks * BLOCK_SIZE;

    //Transform it into a malloc chunk
    auto header = reinterpret_cast<malloc_header_chunk*>(old_end);

    //Update the size
    header->size = real_blocks * BLOCK_SIZE - META_SIZE;

    //Insert the new block into the free list
    insert_after(current, header);

    return true;
}

void init_head(){
    head.size = 0;
    head.next = nullptr;
    head.prev = nullptr;

    malloc_head = reinterpret_cast<malloc_header_chunk*>(&head);
    malloc_head->next = malloc_head;
    malloc_head->prev = malloc_head;

    expand_heap(malloc_head);

    init = true;
}

} //end of anonymous namespace

void* tlib::malloc(size_t bytes){
    if(unlikely(!init)){
        init_head();
    }

    auto current = malloc_head->next;

    //Try not to create too small blocks
    if(bytes < MIN_SPLIT){
        bytes = MIN_SPLIT;
    }

    //Make sure that blocks will have the correct alignment at the end
    if(!aligned(bytes)){
        bytes = ((bytes / ALIGNMENT) + 1) * ALIGNMENT;
    }

    while(true){
        if(current == malloc_head){
            //There are no blocks big enough to hold this request
            //So expand the heap
            expand_heap(current, bytes);
        } else if(current->size >= bytes){
            //This block is big enough

            //Space necessary to hold a new block inside this one
            auto necessary_space = bytes + META_SIZE;

            //Is it worth splitting the block ?
            if(current->size > necessary_space + MIN_SPLIT + META_SIZE){
                auto new_block_size = current->size - necessary_space;

                //Set the new size of the current block
                current->size = bytes;

                //Create a new block inside the current one
                auto new_block = reinterpret_cast<malloc_header_chunk*>(
                    reinterpret_cast<uintptr_t>(current) + bytes + META_SIZE);

                //Update the size of the new block
                new_block->size = new_block_size;

                //Add the new block to the free list
                insert_after(current, new_block);

                //Remove the current block from the free list
                remove(current);

                break;
            } else {
                //Remove this node from the free list
                remove(current);

                break;
            }
        }

        current = current->next;
    }

    _used += current->size + META_SIZE;

    //Address of the start of the block
    auto block_start = reinterpret_cast<uintptr_t>(current) + sizeof(malloc_header_chunk);

    return reinterpret_cast<void*>(block_start);
}

void tlib::free(void* block){
    auto free_header = reinterpret_cast<malloc_header_chunk*>(
        reinterpret_cast<uintptr_t>(block) - sizeof(malloc_header_chunk));

    //Less memory is used
    _used -= free_header->size + META_SIZE;

    //Add the freed block in the free list
    insert_after(malloc_head, free_header);
}

size_t tlib::brk_start(){
    size_t value;
    asm volatile("mov rax, 7; int 50; mov %[brk_start], rax"
        : [brk_start] "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t tlib::brk_end(){
    size_t value;
    asm volatile("mov rax, 8; int 50; mov %[brk_end], rax"
        : [brk_end] "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t tlib::sbrk(size_t inc){
    size_t value;
    asm volatile("mov rax, 9; mov rbx, %[brk_inc]; int 50; mov %[brk_end], rax"
        : [brk_end] "=m" (value)
        : [brk_inc] "g" (inc)
        : "rax", "rbx");
    return value;
}

void* operator new(uint64_t size){
    return tlib::malloc(size);
}

void operator delete(void* p){
    tlib::free(p);
}

void* operator new[](uint64_t size){
    return tlib::malloc(size);
}

void operator delete[](void* p){
    return tlib::free(p);
}
