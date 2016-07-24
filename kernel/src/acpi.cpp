//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "acpica.hpp"
#include "acpi.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"
#include "paging.hpp"
#include "console.hpp"
#include "logging.hpp"
#include "scheduler.hpp"
#include "arch.hpp"

namespace {

// This is copied from acexcep.h
// This could be used with ACPI_DEFINE_EXCEPTION_TABLE but this
// generates too many warnings and errors

static const ACPI_EXCEPTION_INFO AcpiGbl_ExceptionNames_Env[] = {
    EXCEP_TXT ("AE_OK",                         "No error"),
    EXCEP_TXT ("AE_ERROR",                      "Unspecified error"),
    EXCEP_TXT ("AE_NO_ACPI_TABLES",             "ACPI tables could not be found"),
    EXCEP_TXT ("AE_NO_NAMESPACE",               "A namespace has not been loaded"),
    EXCEP_TXT ("AE_NO_MEMORY",                  "Insufficient dynamic memory"),
    EXCEP_TXT ("AE_NOT_FOUND",                  "A requested entity is not found"),
    EXCEP_TXT ("AE_NOT_EXIST",                  "A required entity does not exist"),
    EXCEP_TXT ("AE_ALREADY_EXISTS",             "An entity already exists"),
    EXCEP_TXT ("AE_TYPE",                       "The object type is incorrect"),
    EXCEP_TXT ("AE_NULL_OBJECT",                "A required object was missing"),
    EXCEP_TXT ("AE_NULL_ENTRY",                 "The requested object does not exist"),
    EXCEP_TXT ("AE_BUFFER_OVERFLOW",            "The buffer provided is too small"),
    EXCEP_TXT ("AE_STACK_OVERFLOW",             "An internal stack overflowed"),
    EXCEP_TXT ("AE_STACK_UNDERFLOW",            "An internal stack underflowed"),
    EXCEP_TXT ("AE_NOT_IMPLEMENTED",            "The feature is not implemented"),
    EXCEP_TXT ("AE_SUPPORT",                    "The feature is not supported"),
    EXCEP_TXT ("AE_LIMIT",                      "A predefined limit was exceeded"),
    EXCEP_TXT ("AE_TIME",                       "A time limit or timeout expired"),
    EXCEP_TXT ("AE_ACQUIRE_DEADLOCK",           "Internal error, attempt was made to acquire a mutex in improper order"),
    EXCEP_TXT ("AE_RELEASE_DEADLOCK",           "Internal error, attempt was made to release a mutex in improper order"),
    EXCEP_TXT ("AE_NOT_ACQUIRED",               "An attempt to release a mutex or Global Lock without a previous acquire"),
    EXCEP_TXT ("AE_ALREADY_ACQUIRED",           "Internal error, attempt was made to acquire a mutex twice"),
    EXCEP_TXT ("AE_NO_HARDWARE_RESPONSE",       "Hardware did not respond after an I/O operation"),
    EXCEP_TXT ("AE_NO_GLOBAL_LOCK",             "There is no FACS Global Lock"),
    EXCEP_TXT ("AE_ABORT_METHOD",               "A control method was aborted"),
    EXCEP_TXT ("AE_SAME_HANDLER",               "Attempt was made to install the same handler that is already installed"),
    EXCEP_TXT ("AE_NO_HANDLER",                 "A handler for the operation is not installed"),
    EXCEP_TXT ("AE_OWNER_ID_LIMIT",             "There are no more Owner IDs available for ACPI tables or control methods"),
    EXCEP_TXT ("AE_NOT_CONFIGURED",             "The interface is not part of the current subsystem configuration"),
    EXCEP_TXT ("AE_ACCESS",                     "Permission denied for the requested operation"),
    EXCEP_TXT ("AE_IO_ERROR",                   "An I/O error occurred")
};

volatile bool acpi_initialized = false;

void initialize_acpica(){
    logging::logf(logging::log_level::DEBUG, "acpi:: Started initialization of ACPICA\n");

    /* Initialize the ACPICA subsystem */

    auto status = AcpiInitializeSubsystem();
    if(ACPI_FAILURE(status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to initialize subsystem: error: %s\n", AcpiGbl_ExceptionNames_Env[status].Name);
        return;
    }

    /* Initialize the ACPICA Table Manager and get all ACPI tables */

    status = AcpiInitializeTables(nullptr, 16, true);
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to initialize tables: error: %s\n", AcpiGbl_ExceptionNames_Env[status].Name);
        return;
    }

    /* Create the ACPI namespace from ACPI tables */

    status = AcpiLoadTables ();
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to load tables: error: %s\n", AcpiGbl_ExceptionNames_Env[status].Name);
        return;
    }

    /* Initialize the ACPI hardware */

    status = AcpiEnableSubsystem (ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to enable subsystem: error: %s\n", AcpiGbl_ExceptionNames_Env[status].Name);
        return;
    }

    /* Complete the ACPI namespace object initialization */

    status = AcpiInitializeObjects (ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE (status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to initialize objects: error: %s\n", AcpiGbl_ExceptionNames_Env[status].Name);
        return;
    }

    acpi_initialized = true;

    logging::logf(logging::log_level::DEBUG, "acpi:: Finished initialization of ACPICA\n");
}

} //end of anonymous namespace

void acpi::init(){
    // ACPICA needs scheduling to be started
    scheduler::queue_async_init_task(initialize_acpica);
}

void acpi::shutdown(){
    auto status = AcpiEnterSleepStatePrep(5);

    if(ACPI_FAILURE(status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to prepare sleep state: error: %s\n", AcpiGbl_ExceptionNames_Env[status].Name);
        return;
    }

    size_t rflags;
    arch::disable_hwint(rflags);
    status = AcpiEnterSleepState(5);

    if(ACPI_FAILURE(status)){
        logging::logf(logging::log_level::ERROR, "acpica: Impossible to enter sleep state: error: %s\n", AcpiGbl_ExceptionNames_Env[status].Name);
        return;
    }

    k_print_line("acpi poweroff failed.");
    arch::enable_hwint(rflags);
}

bool acpi::initialized(){
    return acpi_initialized;
}
