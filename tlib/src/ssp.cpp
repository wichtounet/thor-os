//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/print.hpp"
#include "tlib/system.hpp"

extern "C" {

#define STACK_CHK_GUARD 0x595e9fbd94fda766

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn)) void __stack_chk_fail(){
    tlib::printf("Stack smashing detected \n");
    tlib::exit(1);
}

} // end of extern "C"
