#include "thor.hpp"
#include "memory.hpp"

void* operator new (uint64_t size){
    return k_malloc(size);
}

void operator delete (void* p){
    k_free(reinterpret_cast<uint64_t*>(p));
}
