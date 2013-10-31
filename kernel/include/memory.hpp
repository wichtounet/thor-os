#ifndef MEMORY_H
#define MEMORY_H

#include "types.hpp"

struct mmapentry {
    std::size_t base;
    std::size_t size;
    std::size_t type;
};

void load_memory_map();
bool mmap_failed();
std::size_t mmap_entry_count();
const mmapentry& mmap_entry(std::size_t i);

#endif
