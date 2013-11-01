#include "memory.hpp"

namespace {

const std::size_t META_SIZE = sizeof(malloc_header_chunk) + sizeof(malloc_footer_chunk);
const std::size_t MIN_SPLIT = 32;
const std::size_t BLOCK_SIZE = 4096;
const std::size_t MIN_BLOCKS = 4;

struct bios_mmap_entry {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint16_t type;
    uint16_t acpi;
    uint32_t damn_padding;
} __attribute__((packed));

std::size_t e820_failed = 0;
std::size_t entry_count = 0;
bios_mmap_entry* e820_address = 0;

mmapentry e820_mmap[32];

void mmap_query(std::size_t cmd, std::size_t* result){
    std::size_t tmp;
    __asm__ __volatile__ ("mov r8, %0; int 62; mov %1, rax" : : "dN" (cmd), "a" (tmp));
    *result = tmp;
}

struct malloc_header_chunk {
    std::size_t size;
    malloc_header_chunk* next;
    malloc_header_chunk* prev;
};

struct malloc_footer_chunk {
    std::size_t size;
};

struct fake_head {
    std::size_t size;
    malloc_header_chunk* next;
    malloc_header_chunk* prev;
    std::size_t size_2;
};

fake_head head;
malloc_header_chunk* malloc_head = 0;

std::size_t* allocate_block(std::size_t bytes){
    //TODO
}

} //end of anonymous namespace //end of anonymous namespace

void init_memory_manager(){
    //Init the fake head
    head.size = 0;
    head.next = nullptr;
    head.prev = nullptr;
    head.size_2 = 0;

    malloc_head = (malloc_header_chunk*) &head;

    std::size_t* block = allocate_block(MIN_BLOCKS * BLOCK_SIZE);
    malloc_header_chunk* header = (malloc_header_chunk*) block;
    header->size = MIN_BLOCKS * BLOCK_SIZE - META_SIZE;
    header->next  = malloc_head;
    header->prev = malloc_head;

    auto footer = (malloc_footer_chunk*) (block +  header->size);
    footer->size = header->size;

    malloc_head->next = header;
    malloc_head->prev = header;
}

std::size_t* k_malloc(std::size_t bytes){
    std::size_t required_bytes = bytes + META_SIZE;

    malloc_header_chunk* current = malloc_head->next;

    while(true){
        if(current == malloc_head){
            //There are no blocks big enough to hold this request

            std::size_t* block = allocate_block(MIN_BLOCKS * BLOCK_SIZE);
            malloc_header_chunk* header = (malloc_header_chunk*) block;
            header->size = MIN_BLOCKS * BLOCK_SIZE - META_SIZE;

            current->next->prev = header;
            current->next = header;

            header->next  = current->next;
            header->prev = current;

            auto footer = (malloc_footer_chunk*) (block +  header->size);
            footer->size = header->size;
        } else if(current->size >= bytes){
            //This block is big enough

            //Is it worth splitting the block ?
            if(current->size - bytes - META_SIZE > MIN_SPLIT){
                auto old_size = current->size;
                auto new_block_size = current->size - bytes - META_SIZE;

                //Set the new size;
                current->size = bytes;
                ((malloc_footer_chunk*) current + bytes)->size = bytes;

                auto new_block = (malloc_header_chunk*) current + bytes + sizeof(malloc_footer_chunk);

                new_block->size = new_block_size;
                new_block->next = current->next;
                new_block->prev = current->prev;
                current->prev->next = new_block;
                current->next->prev = new_block;

                ((malloc_footer_chunk*) new_block + new_block_size)->size = new_block_size;

                //Make sure the node is clean
                current->prev = nullptr;
                current->next = nullptr;

                return (std::size_t*) current + sizeof(malloc_header_chunk);
            } else {
                //Remove this node from the free list
                current->prev->next = current->next;
                current->next->prev = current->prev;

                //Make sure the node is clean
                current->prev = nullptr;
                current->next = nullptr;

                //The found block can be returned as is
                return (std::size_t*) current + sizeof(malloc_header_chunk);
            }
        }

        current = current->next;
    }
}

void load_memory_map(){
    mmap_query(0, &e820_failed);
    mmap_query(1, &entry_count);
    mmap_query(2, (std::size_t*) &e820_address);

    if(!e820_failed && e820_address){
        for(std::size_t i = 0; i < entry_count; ++i){
            auto& bios_entry = e820_address[i];
            auto& os_entry = e820_mmap[i];

            std::size_t base = bios_entry.base_low + ((std::size_t) bios_entry.base_high << 32);
            std::size_t length = bios_entry.length_low + ((std::size_t) bios_entry.length_high << 32);

            os_entry.base = base;
            os_entry.size = length;
            os_entry.type = bios_entry.type;

            if(os_entry.base == 0 && os_entry.type == 1){
                os_entry.type = 7;
            }
        }
    }
}

std::size_t mmap_entry_count(){
    return entry_count;
}

bool mmap_failed(){
    return e820_failed;
}

const mmapentry& mmap_entry(std::size_t i){
    return e820_mmap[i];
}

const char* str_e820_type(std::size_t type){
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
