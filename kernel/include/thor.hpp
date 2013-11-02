#ifndef THOR_H
#define THOR_H

#include <new>

#include "memory.hpp"

void* operator new (size_t size);
void operator delete (void *p);

#define ATEXIT_MAX_FUNCS	128

extern "C" {

typedef unsigned uarch_t;

struct atexit_func_entry_t {
	/*
	* Each member is at least 4 bytes large. Such that each entry is 12bytes.
	* 128 * 12 = 1.5KB exact.
	**/
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
};

int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

}

#endif
