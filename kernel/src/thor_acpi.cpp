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

/*!
 * \brief Initialize the OSL
 */
ACPI_STATUS AcpiOsInitialize(){
    //Nothing to initialize

    return AE_OK;
}


/*!
 * \brief Terminate the OSL
 */
ACPI_STATUS AcpiOsTerminate(){
    //Nothing to terminate

    return AE_OK;
}

// Overriding of ACPI

/*!
 * \brief Override a predefined ACPI object
 */
ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* /*PredefinedObject*/, ACPI_STRING* new_value){
    *new_value = nullptr;
    return AE_OK;
}

/*!
 * \brief Override an ACPI table
 */
ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* /*ExistingTable*/, ACPI_TABLE_HEADER** new_table){
    *new_table = nullptr;
    return AE_OK;
}

/*!
 * \brief Override a physical ACPI table
 */
ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* /*ExistingTable*/, ACPI_PHYSICAL_ADDRESS* new_address, UINT32* new_table_length){
    *new_address = 0;
    *new_table_length = 0;
    return AE_OK;
}

// Dynamic allocation

/*!
 * \brief Allocate dynamic memory on the heap of the given size.
 */
void* AcpiOsAllocate(ACPI_SIZE size){
    return kalloc::k_malloc(size);
}

/*!
 * \brief Release dynamic memory from the heap.
 */
void AcpiOsFree(void* p){
    kalloc::k_free(p);
}

// terminal

/*!
 * \brief Print something to terminal
 */
void AcpiOsPrintf(const char* format, ...){
    va_list va;
    va_start(va, format);

    printf(format, va);

    va_end(va);
}

/*!
 * \brief Print something to terminal
 */
void AcpiOsVprintf(const char* format, va_list va){
    printf(format, va);
}

/*!
 * \brief Called by the ACPI debugger
 */
ACPI_STATUS AcpiOsSignal(UINT32 function, void* info){
    // This should never happen
    if(!info){
        return AE_NO_MEMORY;
    }

    switch (function) {
        case ACPI_SIGNAL_FATAL:
            {
            auto* fatal_info = static_cast<ACPI_SIGNAL_FATAL_INFO*>(info);

            printf("ACPI fatal signal: Type %h, %h, %h \n", fatal_info->Type, fatal_info->Code, fatal_info->Argument);
            asm volatile("cli; hlt;");

            break;
            }

        case ACPI_SIGNAL_BREAKPOINT:
            printf("ACPI Signal Breakpoint: %s\n", static_cast<char*>(info));
            return AE_NOT_EXIST;

        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

// Scheduling

/*!
 * \brief Return the current thread id
 */
ACPI_THREAD_ID AcpiOsGetThreadId(){
    return scheduler::get_pid();
}

/*
 * \brief Sleep the given number of milliseconds
 */
void AcpiOsSleep(UINT64 ms){
    scheduler::sleep_ms(ms);
}

/*!
 * \brief Active sleep for the given number of microseconds
 */
void AcpiOsStall(UINT32 us){
    //TODO Need a real micro seconds clock

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

/*!
 * \brief Execute the given function in a new process
 */
ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE /*type*/, ACPI_OSD_EXEC_CALLBACK function, void* context){
    auto* user_stack = new char[scheduler::user_stack_size];
    auto* kernel_stack = new char[scheduler::kernel_stack_size];

    auto& process = scheduler::create_kernel_task_args(user_stack, kernel_stack, function, context);
    process.ppid = scheduler::get_pid();

    scheduler::queue_system_process(process.pid);
}

/*!
 * \brief Returns the system time in 100 nanoseconds units
 */
UINT64 AcpiOsGetTimer(){
    //TODO This should be done much more precise
    // and should be real timestamp not an uptime timestamp

    return timer::seconds() * 10000000;
}

/*!
 * \brief Wait for all asynchronous events to complete
 */
void AcpiOsWaitEventsComplete(){
    return;
}

// ACPI

/*!
 * \brief Returns the physical address of the ACPI Root
 */
ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(){
    ACPI_PHYSICAL_ADDRESS  root_pointer;
    root_pointer = 0;
    AcpiFindRootPointer(&root_pointer);
    return root_pointer;
}

// Paging

/*!
 * \brief Map physical memory to a virtual address
 */
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

/*!
 * \brief Unmap physical memory from a virtual address
 */
void AcpiOsUnmapMemory(void* virt_aligned_raw, ACPI_SIZE length){
    size_t pages = (length + paging::PAGE_SIZE - 1) & ~(paging::PAGE_SIZE - 1);

    auto virt_aligned = reinterpret_cast<size_t>(virt_aligned_raw);
    auto virt = virt_aligned - (virt_aligned & ~(paging::PAGE_SIZE - 1));

    paging::unmap_pages(virt, pages);
    virtual_allocator::free(virt, pages);
}

// Concurrency

#if (ACPI_MUTEX_TYPE != ACPI_BINARY_SEMAPHORE)

/*!
 * \brief Create a mutex
 */
ACPI_STATUS AcpiOsCreateMutex(ACPI_MUTEX* handle){
    auto* lock = new mutex<false>();

    lock->init();

    *handle = lock;

    return AE_OK;
}

/*!
 * \brief Delete a mutex
 */
void AcpiOsDeleteMutex(ACPI_MUTEX handle){
    auto* lock = static_cast<mutex<false>*>(handle);

    delete lock;
}

/*!
 * \brief Acquire a mutex
 */
ACPI_STATUS AcpiOsAcquireMutex(ACPI_MUTEX handle, UINT16 Timeout){
    auto* lock = static_cast<mutex<false>*>(handle);

    lock->acquire();

    return AE_OK;
}

/*!
 * \brief Release a mutex
 */
void AcpiOsReleaseMutex(ACPI_MUTEX handle){
    auto* lock = static_cast<mutex<false>*>(handle);

    lock->release();
}

#endif

/*!
 * \brief Create a semaphore
 */
ACPI_STATUS AcpiOsCreateSemaphore(UINT32 /*maxUnits*/, UINT32 initialUnits, ACPI_SEMAPHORE* handle){
    auto* lock = new semaphore();

    lock->init(initialUnits);

    *handle = lock;

    return AE_OK;
}

/*!
 * \brief Delete a semaphore
 */
ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE handle){
    auto* lock = static_cast<semaphore*>(handle);

    delete lock;

    return AE_OK;
}

/*!
 * \brief Wait a semaphore
 */
ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE handle, UINT32 units, UINT16 /*timeout*/){
    auto* lock = static_cast<semaphore*>(handle);

    for(size_t i = 0; i < units; ++i){
        lock->acquire();
    }

    return AE_OK;
}

/*!
 * \brief Signal a semaphore
 */
ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE handle, UINT32 units){
    auto* lock = static_cast<semaphore*>(handle);

    lock->release(units);

    return AE_OK;
}

/*!
 * \brief Create an interrupt spinlock
 */
ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *handle){
    auto* lock = new int_lock();

    *handle = lock;

    return AE_OK;
}

/*!
 * \brief Delete an interrupt spinlock
 */
void AcpiOsDeleteLock(ACPI_HANDLE handle){
    auto* lock = static_cast<semaphore*>(handle);

    delete lock;
}

/*!
 * \brief Acquire an interrupt spinlock
 */
ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK handle){
    auto* lock = static_cast<semaphore*>(handle);

    lock->acquire();

    return 0;
}

/*!
 * \brief Release an interrupt spinlock
 */
void AcpiOsReleaseLock(ACPI_SPINLOCK handle, ACPI_CPU_FLAGS /*flags*/){
    auto* lock = static_cast<semaphore*>(handle);

    lock->release();
}

// Input / Output

/*!
 * \brief Read an hardware
 */
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

/*!
 * \brief Write an hardware
 */
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

/*!
 * \brief Read a physical memory location
 */
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

/*!
 * \brief Write a physical memory location
 */
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
