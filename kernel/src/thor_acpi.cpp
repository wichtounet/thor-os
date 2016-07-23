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
#include "virtual_allocator.hpp"
#include "paging.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"

#include "mutex.hpp"
#include "semaphore.hpp"
#include "int_lock.hpp"

extern "C" {

// Initialization

ACPI_STATUS AcpiOsInitialize(){
    //Nothing to initialize

    return AE_OK;
}

ACPI_STATUS AcpiOsTerminate(){
    //Nothing to terminate

    return AE_OK;
}

// Overriding of ACPI

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* /*PredefinedObject*/, ACPI_STRING* new_value){
    *new_value = nullptr;
    return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* /*ExistingTable*/, ACPI_TABLE_HEADER** new_table){
    *new_table = nullptr;
    return AE_OK;
}

ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* /*ExistingTable*/, ACPI_PHYSICAL_ADDRESS* new_address, UINT32* new_table_length){
    *new_address = 0;
    *new_table_length = 0;
    return AE_OK;
}

// Dynamic allocation

void* AcpiOsAllocate(ACPI_SIZE size){
    return kalloc::k_malloc(size);
}

void AcpiOsFree(void* p){
    kalloc::k_free(p);
}

// terminal

void AcpiOsPrintf(const char* format, ...){
    va_list va;
    va_start(va, format);

    printf(format, va);

    va_end(va);
}

void AcpiOsVprintf(const char* format, va_list va){
    printf(format, va);
}

// Scheduling

ACPI_THREAD_ID AcpiOsGetThreadId(){
    return scheduler::get_pid();
}

//TODO Need a real micro seconds clock
void AcpiOsStall(UINT32 us){
    auto ticks = timer::ticks();
    auto wait = 1 + us / 1000;

    while(timer::ticks() != ticks + wait){
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
}

void AcpiOsSleep(UINT64 ms){
    scheduler::sleep_ms(ms);
}

// ACPI

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(){
    ACPI_PHYSICAL_ADDRESS  root_pointer;
    root_pointer = 0;
    AcpiFindRootPointer(&root_pointer);
    return root_pointer;
}

// Paging

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS phys, ACPI_SIZE length){
    size_t pages = (length + paging::PAGE_SIZE - 1) & ~(paging::PAGE_SIZE - 1);

    auto virt = virtual_allocator::allocate(pages);

    if(!virt){
        return nullptr;
    }

    auto phys_aligned = phys - (phys & ~(paging::PAGE_SIZE - 1));

    if(!paging::map_pages(virt, phys_aligned, pages)){
        return nullptr;
    }

    return reinterpret_cast<void*>(virt + (phys & ~(paging::PAGE_SIZE - 1)));
}

void AcpiOsUnmapMemory(void* virt_aligned_raw, ACPI_SIZE length){
    size_t pages = (length + paging::PAGE_SIZE - 1) & ~(paging::PAGE_SIZE - 1);

    auto virt_aligned = reinterpret_cast<size_t>(virt_aligned_raw);
    auto virt = virt_aligned - (virt_aligned & ~(paging::PAGE_SIZE - 1));

    paging::unmap_pages(virt, pages);
    virtual_allocator::free(virt, pages);
}

// Concurrency

#if (ACPI_MUTEX_TYPE != ACPI_BINARY_SEMAPHORE)

ACPI_STATUS AcpiOsCreateMutex(ACPI_MUTEX* handle){
    auto* lock = new mutex<false>();

    lock->init();

    *handle = lock;

    return AE_OK;
}

void AcpiOsDeleteMutex(ACPI_MUTEX handle){
    auto* lock = static_cast<mutex<false>*>(handle);

    delete lock;
}

ACPI_STATUS AcpiOsAcquireMutex(ACPI_MUTEX handle, UINT16 Timeout){
    auto* lock = static_cast<mutex<false>*>(handle);

    lock->acquire();

    return AE_OK;
}

void AcpiOsReleaseMutex(ACPI_MUTEX handle){
    auto* lock = static_cast<mutex<false>*>(handle);

    lock->release();
}

#endif

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 /*maxUnits*/, UINT32 initialUnits, ACPI_SEMAPHORE* handle){
    auto* lock = new semaphore();

    lock->init(initialUnits);

    *handle = lock;

    return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE handle){
    auto* lock = static_cast<semaphore*>(handle);

    delete lock;

    return AE_OK;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE handle, UINT32 units, UINT16 /*timeout*/){
    auto* lock = static_cast<semaphore*>(handle);

    for(size_t i = 0; i < units; ++i){
        lock->acquire();
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE handle, UINT32 units){
    auto* lock = static_cast<semaphore*>(handle);

    lock->release(units);

    return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *handle){
    auto* lock = new int_lock();

    *handle = lock;

    return AE_OK;
}

void AcpiOsDeleteLock(ACPI_HANDLE handle){
    auto* lock = static_cast<semaphore*>(handle);

    delete lock;
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK handle){
    auto* lock = static_cast<semaphore*>(handle);

    lock->acquire();

    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK handle, ACPI_CPU_FLAGS /*flags*/){
    auto* lock = static_cast<semaphore*>(handle);

    lock->release();
}

// Input / Output

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS port, UINT32* value, UINT32 width){
    switch (width) {
        case 8:
            *value = in_byte(port);
            break;

        case 16:
            *value = in_word(port);
            break;

        case 32:
            *value = in_dword(port);
            break;

        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS port, UINT32 value, UINT32 width){
    switch (width) {
        case 8:
            out_byte(port, value);
            break;

        case 16:
            out_word(port, value);
            break;

        case 32:
            out_dword(port, value);
            break;

        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 *value, UINT32 width){
    ACPI_STATUS rv = AE_OK;

    void* logical_address = AcpiOsMapMemory(Address, width / 8);
    if (!logical_address){
        return AE_NOT_EXIST;
    }

    switch (width) {
        case 8:
            *value = *((volatile uint8_t*) logical_address);
            break;

        case 16:
            *value = *((volatile uint16_t*) logical_address);
            break;

        case 32:
            *value = *((volatile uint32_t*) logical_address);
            break;

        case 64:
            *value = *((volatile uint64_t*) logical_address);
            break;

        default:
            rv = AE_BAD_PARAMETER;
    }

    AcpiOsUnmapMemory(logical_address, width / 8);

    return rv;
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 value, UINT32 width){
    ACPI_STATUS rv = AE_OK;

    void* logical_address = AcpiOsMapMemory(Address, width / 8);
    if (!logical_address){
        return AE_NOT_FOUND;
    }

    switch (width) {
        case 8:
            *((volatile uint8_t*) logical_address) = value;
            break;

        case 16:
            *((volatile uint16_t*) logical_address) = value;
            break;

        case 32:
            *((volatile uint32_t*) logical_address) = value;
            break;

        case 64:
            *((volatile uint64_t*) logical_address) = value;
            break;

        default:
            rv = AE_BAD_PARAMETER;
    }

    AcpiOsUnmapMemory(logical_address, width / 8);

    return rv;
}

} //end of extern "C"
