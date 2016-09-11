//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "arch.hpp"

extern "C" {

void _arch_enable_sse();

} //end of extern "C"

void arch::enable_sse(){
    _arch_enable_sse();
}
