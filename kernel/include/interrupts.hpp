//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

namespace interrupt {

void install_idt();
void install_isrs();

} //end of interrupt namespace

#endif
