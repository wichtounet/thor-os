//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ACPI_HPP
#define ACPI_HPP

namespace acpi {

void init();
bool initialized();

void shutdown();
bool reboot();

} //end of acpi namespace

#endif
