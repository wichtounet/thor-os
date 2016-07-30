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

} //End of anonymous namespace

bool hpet::install(){
    ACPI_TABLE_HPET *hpet;

    // Find the HPET table
    auto status = AcpiGetTable("HPET", 0, (ACPI_TABLE_HEADER **) &hpet);
    if (ACPI_FAILURE(status)){
        return false;
    }

    logging::logf(logging::log_level::TRACE, "hpet: Found ACPI HPET table\n");

    return false;
}
