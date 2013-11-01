#ifndef THOR_H
#define THOR_H

#include <new>

#include "memory.hpp"

void* operator new (size_t size);
void operator delete (void *p);

#endif
