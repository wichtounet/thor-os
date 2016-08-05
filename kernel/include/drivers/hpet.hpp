//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DRIVER_HPET_H
#define DRIVER_HPET_H

namespace hpet {

bool install();
void late_install();

} //end of namespace hpet

#endif
