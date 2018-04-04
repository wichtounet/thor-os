//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "acpica.hpp"
#include "acpi.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"
#include "paging.hpp"
#include "print.hpp"
#include "logging.hpp"
#include "scheduler.hpp"
#include "arch.hpp"
#include "assert.hpp"

namespace {

// This is copied from acexcep.h
// This could be used with ACPI_DEFINE_EXCEPTION_TABLE but this
// generates too many warnings and errors

static const char* AcpiGbl_ExceptionNames_Env[] = {
    "AE_OK",
    "AE_ERROR",
    "AE_NO_ACPI_TABLES",
    "AE_NO_NAMESPACE",
    "AE_NO_MEMORY",
    "AE_NOT_FOUND",
    "AE_NOT_EXIST",
    "AE_ALREADY_EXISTS",
    "AE_TYPE",
    "AE_NULL_OBJECT",
    "AE_NULL_ENTRY",
    "AE_BUFFER_OVERFLOW",
    "AE_STACK_OVERFLOW",
    "AE_STACK_UNDERFLOW",
    "AE_NOT_IMPLEMENTED",
    "AE_SUPPORT",
    "AE_LIMIT",
    "AE_TIME",
    "AE_ACQUIRE_DEADLOCK",
    "AE_RELEASE_DEADLOCK",
    "AE_NOT_ACQUIRED",
    "AE_ALREADY_ACQUIRED",
    "AE_NO_HARDWARE_RESPONSE",
    "AE_NO_GLOBAL_LOCK",
    "AE_ABORT_METHOD",
    "AE_SAME_HANDLER",
    "AE_NO_HANDLER",
    "AE_OWNER_ID_LIMIT",
    "AE_NOT_CONFIGURED",
    "AE_ACCESS",
    "AE_IO_ERROR"
};

volatile bool acpi_initialized = false;

void initialize_acpica(){
    logging::logf(logging::log_level::DEBUG, "acpi: Started initialization of ACPICA\n");

    /* Initialize the ACPICA subsystem */

    auto status = AcpiInitializeSubsystem();
    if(ACPI_FAILURE(status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to initialize subsystem: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        return;
    }

    /* Initialize the ACPICA Table Manager and get all ACPI tables */

    status = AcpiInitializeTables(nullptr, 16, true);
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to initialize tables: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        return;
    }

    /* Create the ACPI namespace from ACPI tables */

    status = AcpiLoadTables ();
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to load tables: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        return;
    }

    /* Initialize the ACPI hardware */

    status = AcpiEnableSubsystem (ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to enable subsystem: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        return;
    }

    /* Complete the ACPI namespace object initialization */

    status = AcpiInitializeObjects (ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to initialize objects: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        return;
    }

    acpi_initialized = true;

    logging::logf(logging::log_level::DEBUG, "acpi: Finished initialization of ACPICA\n");
}

uint64_t acpi_read(const ACPI_GENERIC_ADDRESS& address){
    if(address.SpaceId == ACPI_ADR_SPACE_SYSTEM_MEMORY){
        UINT64 value = 0;
        auto status = AcpiOsReadMemory(address.Address, &value, address.BitWidth);
        if(ACPI_FAILURE(status)){
            logging::logf(logging::log_level::ERROR, "acpica: Unable to read from memory: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        }
        return value;
    } else if(address.SpaceId == ACPI_ADR_SPACE_SYSTEM_IO){
        UINT32 value = 0;
        auto status = AcpiHwReadPort(address.Address, &value, address.BitWidth);
        if(ACPI_FAILURE(status)){
            logging::logf(logging::log_level::ERROR, "acpica: Unable to read from hardware port: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        }
        return value;
    } else {
        logging::logf(logging::log_level::ERROR, "acpica: Unimplemented read generic address space id\n");
        return 0;
    }
}

void acpi_write(const ACPI_GENERIC_ADDRESS& address, uint64_t value){
    if(address.SpaceId == ACPI_ADR_SPACE_SYSTEM_MEMORY){
        auto status = AcpiOsWriteMemory(address.Address, value, address.BitWidth);
        if(ACPI_FAILURE(status)){
            logging::logf(logging::log_level::ERROR, "acpica: Unable to write to memory: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        }
    } else if(address.SpaceId == ACPI_ADR_SPACE_SYSTEM_IO){
        auto status = AcpiHwWritePort(address.Address, value, address.BitWidth);
        if(ACPI_FAILURE(status)){
            logging::logf(logging::log_level::ERROR, "acpica: Unable to write to hardware port: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        }
    } else {
        logging::logf(logging::log_level::ERROR, "acpica: Unimplemented write generic address space id\n");
    }
}

} //end of anonymous namespace

void acpi::init(){
    // ACPICA needs scheduling to be started
    scheduler::queue_async_init_task(initialize_acpica);
}

bool acpi::initialized(){
    return acpi_initialized;
}

void acpi::shutdown(){
    thor_assert(acpi::initialized(), "ACPI must be initialized for acpi::shutdown()");

    auto status = AcpiEnterSleepStatePrep(5);

    if(ACPI_FAILURE(status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to prepare sleep state: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        return;
    }

    size_t rflags;
    arch::disable_hwint(rflags);
    status = AcpiEnterSleepState(5);

    if(ACPI_FAILURE(status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to enter sleep state: error: %s\n", AcpiGbl_ExceptionNames_Env[status]);
        return;
    }

    k_print_line("acpi poweroff failed.");
    arch::enable_hwint(rflags);
}

bool acpi::reboot(){
    thor_assert(acpi::initialized(), "ACPI must be initialized for acpi::reboot()");

    if (AcpiGbl_FADT.Header.Revision < FADT2_REVISION_ID){
        return false;
    }

    if(!(AcpiGbl_FADT.Flags & ACPI_FADT_RESET_REGISTER)){
        return false;
    }

    auto reset_register = AcpiGbl_FADT.ResetRegister;
    auto reset_value = AcpiGbl_FADT.ResetValue;

    acpi_write(reset_register, reset_value);

    return true;
}
