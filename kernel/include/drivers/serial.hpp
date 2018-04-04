//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SERIAL_H
#define SERIAL_H

namespace serial {

void init();

bool is_transmit_buffer_empty();
void transmit(char a);

} //end of serial namespace

#endif
