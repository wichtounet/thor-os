//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "thor.hpp"
#include "kalloc.hpp"
#include "scheduler.hpp"
#include "logging.hpp"
#include "print.hpp"

void* operator new(uint64_t size){
    return kalloc::k_malloc(size);
}

void operator delete(void* p){
    kalloc::k_free(p);
}

void* operator new[](uint64_t size){
    return kalloc::k_malloc(size);
}

void operator delete[](void* p){
    return kalloc::k_free(p);
}

extern "C" {

atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
uarch_t __atexit_func_count = 0;

int __cxa_atexit(void (*f)(void *), void *objptr, void *dso){
    if (__atexit_func_count >= ATEXIT_MAX_FUNCS) {
        return -1;
    }

    __atexit_funcs[__atexit_func_count].destructor_func = f;
    __atexit_funcs[__atexit_func_count].obj_ptr = objptr;
    __atexit_funcs[__atexit_func_count].dso_handle = dso;
    __atexit_func_count++;

    return 0; /*I would prefer if functions returned 1 on success, but the ABI says...*/
}

void __cxa_finalize(void *f){
    uarch_t i = __atexit_func_count;
    if (!f){
        while (i--){
            if (__atexit_funcs[i].destructor_func){
                (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            }
        }
        return;
    }

    for ( ; i >= 0; --i){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        if (__atexit_funcs[i].destructor_func == f){
            (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            __atexit_funcs[i].destructor_func = 0;
        }
#pragma GCC diagnostic pop
    }
}

#define STACK_CHK_GUARD 0x595e9fbd94fda766

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

#define double_printf(...) \
    logging::logf(logging::log_level::ERROR, __VA_ARGS__); \
    printf(__VA_ARGS__);

__attribute__((noreturn)) void __stack_chk_fail(){
    double_printf("Stack smashing detected \n");
    double_printf("pid=%u\n", scheduler::get_pid());
    asm volatile("hlt" : : );
    __builtin_unreachable();
}

}
