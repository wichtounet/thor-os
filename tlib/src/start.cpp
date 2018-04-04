//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/system.hpp"

int main(int argc, char* argv[]);

extern "C" {

void __cxa_finalize(void* f);

void _init();
void _fini();

void _start(int argc, char* argv[]) __attribute__((section(".start")));

void _start(int argc, char* argv[]){
    // Call the global constructors, if any
    _init();

    // Execute the main function
    auto code = main(argc, argv);

    // Call the global destructors, if any
    _fini();

    // Call the shared library destructors, if any
    __cxa_finalize(nullptr);

    // Kill the process with the correct exit code
    tlib::exit(code);
}

#define ATEXIT_MAX_FUNCS 32

struct atexit_func_entry_t {
    void (*destructor_func)(void *);
    void *obj_ptr;
    void *dso_handle;
};

atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
int __atexit_func_count = 0;

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

void __cxa_finalize(void* f){
    int i = __atexit_func_count;
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

} // end of extern C
