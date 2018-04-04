//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

/*
 * This file contains the OS specific layer
 */

#include "conc/mutex.hpp"
#include "conc/semaphore.hpp"
#include "conc/int_lock.hpp"

#include "acpica.hpp"

#include "kalloc.hpp"
#include "print.hpp"
#include "scheduler.hpp"
#include "virtual_allocator.hpp"
#include "paging.hpp"
#include "kernel_utils.hpp"
#include "interrupts.hpp"
#include "timer.hpp"
#include "drivers/pci.hpp"
#include "logging.hpp"

#ifdef THOR_CONFIG_ACPI_OSL_VERBOSE
#define verbose_logf(...) logging::logf(__VA_ARGS__)
#else
#define verbose_logf(...)
#endif

extern "C" {

// Initialization

/*!
 * \brief Initialize the OSL
 */
ACPI_STATUS AcpiOsInitialize(){
    logging::logf(logging::log_level::DEBUG, "ACPICA started initialization\n");

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
    auto memory = kalloc::k_malloc(size);

    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: Allocate(size=%u)=%p\n", size, memory);

    return memory;
}

/*!
 * \brief Release dynamic memory from the heap.
 */
void AcpiOsFree(void* p){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: Free(p=%p)\n", p);

    kalloc::k_free(p);
}

// terminal

/*!
 * \brief Print something to terminal
 */
void AcpiOsPrintf(const char* format, ...){
    va_list va;
    va_start(va, format);

    logging::logf(logging::log_level::TRACE, format, va);

    va_end(va);
}

/*!
 * \brief Print something to terminal
 */
void AcpiOsVprintf(const char* format, va_list va){
    logging::logf(logging::log_level::TRACE, format, va);
}

/*!
 * \brief Called by the ACPI debugger
 */
ACPI_STATUS AcpiOsSignal(UINT32 function, void* info){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: Signal\n");

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
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: GetThreadId\n");

    return scheduler::get_pid();
}

/*
 * \brief Sleep the given number of milliseconds
 */
void AcpiOsSleep(UINT64 ms){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: Sleep\n");

    scheduler::sleep_ms(ms);
}

/*!
 * \brief Active sleep for the given number of microseconds
 */
void AcpiOsStall(UINT32 us){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: Stall\n");

    uint64_t c = timer::counter();
    uint64_t wait = us * (timer::counter_frequency() / double(1000000));
    wait = !wait ? 1 : wait;

    while(timer::counter() != c + wait){
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
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: Execute\n");

    auto* user_stack = new char[scheduler::user_stack_size];
    auto* kernel_stack = new char[scheduler::kernel_stack_size];

    auto& process = scheduler::create_kernel_task_args("acpi", user_stack, kernel_stack, function, context);
    process.ppid = scheduler::get_pid();

    logging::logf(logging::log_level::DEBUG, "ACPICA new process %u\n", process.pid);

    scheduler::queue_system_process(process.pid);

    return AE_OK;
}

/*!
 * \brief Returns the system time in 100 nanoseconds units
 */
UINT64 AcpiOsGetTimer(){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: GetTimer\n");

    //TODO This should be done much more precise
    // and should be real timestamp not an uptime timestamp

    return timer::seconds() * 10000000;
}

/*!
 * \brief Wait for all asynchronous events to complete
 */
void AcpiOsWaitEventsComplete(){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: WaitEventsComplete\n");

    return;
}

// ACPI

/*!
 * \brief Returns the physical address of the ACPI Root
 */
ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: GetRootPointer\n");

    ACPI_PHYSICAL_ADDRESS  root_pointer;
    root_pointer = 0;
    auto status = AcpiFindRootPointer(&root_pointer);
    if(ACPI_FAILURE(status)){
        logging::logf(logging::log_level::ERROR, "acpica: Unable to find ACPI root pointer: error: %u\n", size_t(status));
    } else {
        logging::logf(logging::log_level::TRACE, "acpica: ACPI Root pointer found at physical address %h\n", size_t(root_pointer));
    }
    return root_pointer;
}

// Paging

/*!
 * \brief Map physical memory to a virtual address
 */
void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS phys, ACPI_SIZE length){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: MapMemory\n");

    auto offset = phys % paging::PAGE_SIZE;

    auto real_length = offset + length;
    auto pages = paging::pages(real_length);

    auto virt_aligned = virtual_allocator::allocate(pages);

    if(!virt_aligned){
        logging::logf(logging::log_level::ERROR, "acpica: map memory failed (impossible to allocate virtual memory\n");
        return nullptr;
    }

    auto phys_aligned = phys - offset;

    if(!paging::map_pages(virt_aligned, phys_aligned, pages)){
        logging::logf(logging::log_level::ERROR, "acpica: map memory failed (impossible to map pages\n");
        return nullptr;
    }

    return reinterpret_cast<void*>(virt_aligned + offset);
}

/*!
 * \brief Unmap physical memory from a virtual address
 */
void AcpiOsUnmapMemory(void* virt_raw, ACPI_SIZE length){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: UnmapMemory\n");

    auto virt = reinterpret_cast<size_t>(virt_raw);

    auto offset = virt % paging::PAGE_SIZE;
    auto real_length = offset + length;
    auto pages = paging::pages(real_length);

    auto virt_aligned = virt - offset;

    paging::unmap_pages(virt_aligned, pages);
    virtual_allocator::free(virt_aligned, pages);
}

// Concurrency

#if (ACPI_MUTEX_TYPE != ACPI_BINARY_SEMAPHORE)

/*!
 * \brief Create a mutex
 */
ACPI_STATUS AcpiOsCreateMutex(ACPI_MUTEX* handle){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: CreateMutex\n");

    auto* lock = new mutex<false>();

    lock->init();

    *handle = lock;

    return AE_OK;
}

/*!
 * \brief Delete a mutex
 */
void AcpiOsDeleteMutex(ACPI_MUTEX handle){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: DeleteMutex\n");

    auto* lock = static_cast<mutex<false>*>(handle);

    delete lock;
}

/*!
 * \brief Acquire a mutex
 */
ACPI_STATUS AcpiOsAcquireMutex(ACPI_MUTEX handle, UINT16 Timeout){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: AcquireMutex\n");

    auto* lock = static_cast<mutex<false>*>(handle);

    lock->lock();

    return AE_OK;
}

/*!
 * \brief Release a mutex
 */
void AcpiOsReleaseMutex(ACPI_MUTEX handle){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: ReleaseMutex\n");

    auto* lock = static_cast<mutex<false>*>(handle);

    lock->unlock();
}

#endif

/*!
 * \brief Create a semaphore
 */
ACPI_STATUS AcpiOsCreateSemaphore(UINT32 /*maxUnits*/, UINT32 initialUnits, ACPI_SEMAPHORE* handle){
    auto* lock = new semaphore();

    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: CreateSemaphore(initial=%u)=%p\n", initialUnits, lock);

    lock->init(initialUnits);

    *handle = lock;

    return AE_OK;
}

/*!
 * \brief Delete a semaphore
 */
ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE handle){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: DeleteSemaphore\n");

    auto* lock = static_cast<semaphore*>(handle);

    delete lock;

    return AE_OK;
}

/*!
 * \brief Wait a semaphore
 */
ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE handle, UINT32 units, UINT16 timeout){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: WaitSemaphore(handle=%p,units=%u,timeout=%u)\n", handle, units, timeout);

    auto* lock = static_cast<semaphore*>(handle);

    for(size_t i = 0; i < units; ++i){
        lock->lock();
    }

    return AE_OK;
}

/*!
 * \brief Signal a semaphore
 */
ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE handle, UINT32 units){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: SignalSemaphore\n");

    auto* lock = static_cast<semaphore*>(handle);

    lock->release(units);

    return AE_OK;
}

/*!
 * \brief Create an interrupt spinlock
 */
ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *handle){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: CreateLock\n");

    auto* lock = new int_lock();

    *handle = lock;

    return AE_OK;
}

/*!
 * \brief Delete an interrupt spinlock
 */
void AcpiOsDeleteLock(ACPI_HANDLE handle){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: DeleteLock\n");

    auto* lock = static_cast<int_lock*>(handle);

    delete lock;
}

/*!
 * \brief Acquire an interrupt spinlock
 */
ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK handle){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: acquireLock\n");

    auto* lock = static_cast<int_lock*>(handle);

    lock->lock();

    return 0;
}

/*!
 * \brief Release an interrupt spinlock
 */
void AcpiOsReleaseLock(ACPI_SPINLOCK handle, ACPI_CPU_FLAGS /*flags*/){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: ReleaseLock\n");

    auto* lock = static_cast<int_lock*>(handle);

    lock->unlock();
}

// Input / Output

/*!
 * \brief Read an hardware
 */
ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS port, UINT32* value, UINT32 width){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: ReadPort\n");

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
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: WritePort\n");

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
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: ReadMemory\n");

    ACPI_STATUS rv = AE_OK;

    void* logical_address = AcpiOsMapMemory(Address, width / 8);
    if (!logical_address){
        logging::log(logging::log_level::ERROR, "acpica: Cannot read memory (cannot map memory)\n");
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
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: WriteMemory\n");

    ACPI_STATUS rv = AE_OK;

    void* logical_address = AcpiOsMapMemory(Address, width / 8);
    if (!logical_address){
        logging::log(logging::log_level::ERROR, "acpica: Cannot write memory (cannot map memory)\n");
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

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* pci_id, UINT32 Register, UINT64* value, UINT32 width){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: ReadPciConfiguration\n");

    if (pci_id->Bus >= 256 || pci_id->Device >= 32 || pci_id->Function >= 8){
        return AE_BAD_PARAMETER;
    }

    switch (width) {
        case 8:
            *((uint8_t*) value) = pci::read_config_byte(pci_id->Bus, pci_id->Device, pci_id->Function, Register);
            break;

        case 16:
            *((uint16_t*) value) = pci::read_config_word(pci_id->Bus, pci_id->Device, pci_id->Function, Register);
            break;

        case 32:
            *((uint32_t*) value) = pci::read_config_dword(pci_id->Bus, pci_id->Device, pci_id->Function, Register);
            break;

        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* pci_id, UINT32 Register, ACPI_INTEGER value, UINT32 width){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: WritePciConfiguration\n");

    if (pci_id->Bus >= 256 || pci_id->Device >= 32 || pci_id->Function >= 8){
        return AE_BAD_PARAMETER;
    }

    switch (width) {
        case 8:
            pci::write_config_byte(pci_id->Bus, pci_id->Device, pci_id->Function, Register, value);
            break;

        case 16:
            pci::write_config_word(pci_id->Bus, pci_id->Device, pci_id->Function, Register, value);
            break;

        case 32:
            pci::write_config_dword(pci_id->Bus, pci_id->Device, pci_id->Function, Register, value);
            break;

        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

// Interrupts

struct acpi_interrupt_context {
    ACPI_OSD_HANDLER routine;
    void* context;
};

void acpi_interrupt_handler(interrupt::syscall_regs*, void* context){
    auto acpi_context = static_cast<acpi_interrupt_context*>(context);
    acpi_context->routine(acpi_context->context);
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 irq, ACPI_OSD_HANDLER routine, void* context){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: InstallInterruptHandler\n");

    if (irq > 255){
        return AE_BAD_PARAMETER;
    }

    if (!routine){
        return AE_BAD_PARAMETER;
    }

    //TODO This memory is leaking
    //Needs to be removed in acpiosremoveinterrupthandler

    auto* acpi_context = new acpi_interrupt_context();
    acpi_context->routine = routine;
    acpi_context->context = context;

    if(!interrupt::register_irq_handler(irq, acpi_interrupt_handler, acpi_context)){
        logging::logf(logging::log_level::ERROR, "acpica: cannot install interrupt handler\n");
        return AE_ALREADY_EXISTS;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 irq, ACPI_OSD_HANDLER routine){
    verbose_logf(logging::log_level::TRACE, "thor:acpica:osl: RemoveInterruptHandler\n");

    if (irq > 255){
        return AE_BAD_PARAMETER;
    }

    if (!routine){
        return AE_BAD_PARAMETER;
    }

    if(!interrupt::unregister_irq_handler(irq, acpi_interrupt_handler)){
        logging::log(logging::log_level::ERROR, "acpica: Cannot remove interrupt handler\n");
        return AE_NOT_EXIST;
    }

    return AE_OK;
}

} //end of extern "C"
