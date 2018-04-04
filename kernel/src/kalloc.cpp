//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "kalloc.hpp"
#include "print.hpp"
#include "physical_allocator.hpp"
#include "paging.hpp"
#include "e820.hpp"
#include "logging.hpp"
#include "assert.hpp"

#include "conc/int_lock.hpp"

#include "fs/sysfs.hpp"

namespace {

constexpr size_t ALIGNMENT = 16;

constexpr bool aligned(size_t value){
    return (value & (ALIGNMENT - 1)) == 0;
}

//Used to compile with malloc operations in the console
//can produce a lot of output
constexpr bool DEBUG_MALLOC      = false;
constexpr bool DEBUG_MALLOC_FULL = false;
constexpr bool TRACE_MALLOC      = false;

size_t _used_memory;
size_t _allocations;
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

    malloc_footer_chunk* footer() const {
        thor_assert(aligned(reinterpret_cast<uintptr_t>(static_cast<const volatile void*>(this))), "kalloc: Invalid alignment");

        return reinterpret_cast<malloc_footer_chunk*>(
            reinterpret_cast<uintptr_t>(static_cast<const volatile void*>(this)) + __size + sizeof(malloc_header_chunk));
    }
};

struct malloc_footer_chunk {
private:
    uint64_t __size;

public:
    uint64_t& size(){
        return __size;
    }
};

struct fake_head {
    uint64_t size;
    malloc_header_chunk* next;
    malloc_header_chunk* prev;
    uint32_t _free;
    uint16_t left;
    uint16_t right;
    uint64_t size_2; // This is the size of the footer
};

constexpr const size_t HEADER_SIZE = sizeof(malloc_header_chunk);
constexpr const size_t FOOTER_SIZE = sizeof(malloc_footer_chunk);
constexpr const size_t META_SIZE = HEADER_SIZE + FOOTER_SIZE;
constexpr const size_t BLOCK_SIZE = paging::PAGE_SIZE;
constexpr const size_t MIN_BLOCKS = 4;

constexpr const uint64_t MIN_SPLIT_BASE = ALIGNMENT == 8 ? 32 : 2 * ALIGNMENT;
constexpr const uint64_t MIN_SPLIT = MIN_SPLIT_BASE - sizeof(malloc_footer_chunk);

static_assert(aligned(sizeof(malloc_header_chunk)), "The header must be aligned");
static_assert(aligned(MIN_SPLIT + sizeof(malloc_footer_chunk)), "The size of minimum split must guarantee alignment");
static_assert(sizeof(malloc_footer_chunk) == sizeof(uint64_t), "The footer must a single quadword");
static_assert(sizeof(fake_head) == META_SIZE, "The fake head must be the sum of header and footer");

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
        // TODO Fix the leak
        return nullptr;
    }

    //Map the physical memory at the virtual address
    if(!paging::map_pages(virtual_memory, physical_memory, blocks)){
        // TODO Fix the leak
        return nullptr;
    }

    _allocated_memory += blocks * BLOCK_SIZE;

    return reinterpret_cast<uint64_t*>(virtual_memory);
}

void check_node(malloc_header_chunk* node, const char* point = nullptr){
    if(node->size() != node->footer()->size()){
        logging::logf(logging::log_level::ERROR, "Inconsistent header/footer size for block %p (%s)\n", node, point);
    }

    if(node->size() > 1024UL * 1024UL * 1024UL * 1024UL * 1024UL){
        logging::logf(logging::log_level::ERROR, "Invalid header size for block %p:%u (%s)\n", node, node->size(), point);
    }

    if(node->footer()->size() > 1024UL * 1024UL * 1024UL * 1024UL * 1024UL){
        logging::logf(logging::log_level::ERROR, "Invalid footer size for block %p:%u (%s)\n", node, node->footer()->size(), point);
    }

    if(reinterpret_cast<uintptr_t>(node->footer()) != reinterpret_cast<uintptr_t>(node) + node->size() + HEADER_SIZE){
        logging::logf(logging::log_level::ERROR, "Inconsistent footer address for block %p\n (%s)", node, point);
    }
}

void debug_malloc(const char* point = nullptr){
    if(DEBUG_MALLOC || DEBUG_MALLOC_FULL){
        // 0. Check for inconsistencies

        {
            auto it = malloc_head;

            do {
                check_node(it, point);

                it = it->next();
            } while(it != malloc_head);
        }

        if (DEBUG_MALLOC_FULL) {
            if(point){
                logging::log(logging::log_level::DEBUG, point);
            }

            {
                auto it = malloc_head;

                logging::log("\n next: ");
                do {
                    logging::logf("%u:%h(%u) -> ", static_cast<size_t>(it->is_free()), reinterpret_cast<uint64_t>(it), it->size());
                    it = it->next();
                } while(it != malloc_head);

                logging::logf("%h", malloc_head);
            }

            {
                auto it = malloc_head;

                logging::log("\n prev: ");
                do {
                    logging::logf("%u:%h(%u) <- ", static_cast<size_t>(it->is_free()), reinterpret_cast<uint64_t>(it), it->size());
                    it = it->prev();
                } while(it != malloc_head);

                logging::logf("%h\n", malloc_head);
            }
        }
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

void expand_heap(malloc_header_chunk* current, uint64_t bytes = 0){
    auto blocks = MIN_BLOCKS;

    if(bytes){
        auto necessary_blocks = ((bytes + META_SIZE) / BLOCK_SIZE) + 1;

        if(necessary_blocks > blocks){
            blocks = necessary_blocks;
        }
    }

    //Allocate a new block of memory
    uint64_t* block = allocate_block(blocks);

    thor_assert(block, "Impossible to allocate block for expand_heap");

    //Transform it into a malloc chunk
    auto header = reinterpret_cast<malloc_header_chunk*>(block);

    //Update the sizes
    auto size = blocks * BLOCK_SIZE - META_SIZE;
    header->size() = size;
    header->footer()->size() = size;

    //When created a block has no left and right neighbour
    header->set_left(false);
    header->set_right(false);

    //Insert the new block into the free list
    insert_after(current, header);

    check_node(header, "header after expand_head");
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

        check_node(b, "b after coalesce");
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

        check_node(b, "b after coalesce");
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

std::string sysfs_allocations(){
    return std::to_string(kalloc::allocations());
}

} //end of anonymous namespace

void kalloc::init(){
    // Set the counters
    _used_memory = 0;
    _allocations = 0;
    _allocated_memory = 0;

    //Init the fake head
    init_head();

    //Allocate a first block
    expand_heap(malloc_head);
}

void kalloc::finalize(){
    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/memory/dynamic/free"), &sysfs_free);
    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/memory/dynamic/used"), &sysfs_used);
    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/memory/dynamic/allocated"), &sysfs_allocated);
    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/memory/dynamic/allocations"), &sysfs_allocations);
}

void* kalloc::k_malloc(uint64_t bytes){
    direct_int_lock l;

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
                    reinterpret_cast<uintptr_t>(static_cast<const void*>(current)) + bytes + META_SIZE);

                //Update the size of the new block
                new_block->size() = new_block_size;
                new_block->footer()->size() = new_block_size;

                new_block->set_left(true);
                new_block->set_right(block_right);

                //Add the new block to the free list
                insert_after(current, new_block);

                //Remove the current block from the free list
                remove(current);

                check_node(current, "current after malloc split");
                check_node(new_block, "new_block after malloc split");

                debug_malloc("after malloc split");

                break;
            } else {
                //Remove this node from the free list
                remove(current);

                check_node(current, "current after malloc no split");

                debug_malloc("after malloc no split");

                break;
            }
        }

        current = current->next();
    }

    _used_memory += current->size() + META_SIZE;
    ++_allocations;

    //Address of the start of the block
    auto block_start = reinterpret_cast<uintptr_t>(current) + sizeof(malloc_header_chunk);

    if(TRACE_MALLOC){
        logging::logf(logging::log_level::TRACE, "m %u(%u) %h\n", bytes, current->size(), block_start);
    }

    return reinterpret_cast<void*>(block_start);
}

void kalloc::k_free(void* block){
    direct_int_lock lock;

    auto free_header = reinterpret_cast<malloc_header_chunk*>(
        reinterpret_cast<uintptr_t>(block) - sizeof(malloc_header_chunk));

    thor_assert(!free_header->is_free(), "kalloc: free block getting freed\n");

    if(TRACE_MALLOC){
        logging::logf(logging::log_level::TRACE, "f %u %h\n", free_header->size(), reinterpret_cast<uint64_t>(block));
    }

    //Less memory is used
    _used_memory -= free_header->size() + META_SIZE;

    //Coalesce the block if possible
    free_header = coalesce(free_header);

    //Add the freed block in the free list
    insert_after(malloc_head, free_header);

    check_node(free_header, "block after free");

    debug_malloc("after free");
}

size_t kalloc::allocated_memory(){
    return _allocated_memory;
}

size_t kalloc::used_memory(){
    return _used_memory;
}

size_t kalloc::allocations(){
    return _allocations;
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
