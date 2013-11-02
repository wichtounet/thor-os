#include "thor.hpp"
#include "memory.hpp"

void* operator new (size_t size){
    return k_malloc(size);
}

void operator delete (void* p){
    k_free(reinterpret_cast<std::size_t*>(p));
}
