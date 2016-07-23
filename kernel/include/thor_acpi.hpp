//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef THOR_ACPI_HPP
#define THOR_ACPI_HPP

// This file contains the OS specific layer for ACPICA for thor-os
// It is meant to only be included by thor_acenv

//thor works in 64 bits
#define ACPI_MACHINE_WIDTH 64

// Cannot compile ACPICA without this flag for some reason
#define ACPI_DEBUGGER

// Let APCICA use its own cache
#define ACPI_USE_LOCAL_CACHE

// Limit compatibility to ACPI 5.0
//#define ACPI_REDUCED_HARDWARE TRUE

#endif
