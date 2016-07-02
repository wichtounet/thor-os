//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SERIAL_H
#define SERIAL_H

namespace serial {

void init();

bool is_transmit_buffer_empty();
void transmit(char a);

} //end of serial namespace

#endif
