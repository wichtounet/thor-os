//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "drivers/hpet.hpp"

#include "acpica.hpp"
#include "logging.hpp"

namespace {

ACPI_TABLE_HPET* hpet_table;

} //End of anonymous namespace

bool hpet::install(){
    // Find the HPET table
    auto status = AcpiGetTable(ACPI_SIG_HPET, 0, (ACPI_TABLE_HEADER **) &hpet_table);
    if (ACPI_FAILURE(status)){
        return false;
    }

    logging::logf(logging::log_level::TRACE, "hpet: Found ACPI HPET table\n");

    return true;
}

void hpet::late_install(){
    if(hpet::install()){
        logging::logf(logging::log_level::TRACE, "hpet: Late install suceeded\n");

        logging::logf(logging::log_level::TRACE, "hpet: HPET Address: %h\n", hpet_table->Address.Address);

        //TODO Register the timer to the timer system
    }
}
