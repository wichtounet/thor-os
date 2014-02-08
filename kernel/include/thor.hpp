//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef THOR_H
#define THOR_H

#include <types.hpp>

void* operator new(uint64_t size);
void operator delete(void* p);

void* operator new[](uint64_t size);
void operator delete[](void* p);

#define ATEXIT_MAX_FUNCS 32

extern "C" {

typedef int uarch_t;

struct atexit_func_entry_t {
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
};

int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

}

#endif
