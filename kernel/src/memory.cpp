#include "memory.hpp"

std::size_t e820_failed = 0;
std::size_t entry_count = 0;
mmapentry* e820_address = 0;

mmapentry e820_mmap[32];

void mmap_query(std::size_t cmd, std::size_t* result){
    std::size_t tmp;
    __asm__ __volatile__ ("mov r8, %0; int 62; mov %1, rax" : : "dN" (cmd), "a" (tmp));
    *result = tmp;
}

void load_memory_map(){
    mmap_query(0, &e820_failed);
    mmap_query(1, &entry_count);
    mmap_query(2, (std::size_t*) &e820_address);

    if(!e820_failed && e820_address){
        for(std::size_t i = 0; i < entry_count; ++i){
            e820_mmap[i] = e820_address[i];
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
