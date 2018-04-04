//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef THOR_ACPI_HPP
#define THOR_ACPI_HPP

// This file contains the OS specific layer for ACPICA for thor-os
// It is meant to only be included by thor_acenv

//thor works in 64 bits
#define ACPI_MACHINE_WIDTH 64

// Don't use the full debugger, only the features for debugging output
#define ACPI_DEBUG_OUTPUT

// Let APCICA use its own cache
#define ACPI_USE_LOCAL_CACHE

// Limit compatibility to ACPI 5.0
#define ACPI_REDUCED_HARDWARE TRUE

#endif
