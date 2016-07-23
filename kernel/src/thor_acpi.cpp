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
#include "scheduler.hpp"

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

ACPI_THREAD_ID AcpiOsGetThreadId(void){
    return scheduler::get_pid();
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(){
    ACPI_PHYSICAL_ADDRESS  root_pointer;
    root_pointer = 0;
    AcpiFindRootPointer(&root_pointer);
    return root_pointer;
}

} //end of extern "C"
