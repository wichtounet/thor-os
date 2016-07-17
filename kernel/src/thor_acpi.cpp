//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
 * This file contains the OS specific layer
 */

#include "acpica.hpp"

#include "kalloc.hpp"
#include "console.hpp"

extern "C" {

void* AcpiOsAllocate(ACPI_SIZE size){
    return kalloc::k_malloc(size);
}

void AcpiOsFree(void* p){
    kalloc::k_free(p);
}

void AcpiOsPrintf(const char* format, ...){
    va_list va;
    va_start(va, format);

    printf(format, va);

    va_end(va);
}

void AcpiOsVprintf(const char* format, va_list va){
    printf(format, va);
}

} //end of extern "C"
