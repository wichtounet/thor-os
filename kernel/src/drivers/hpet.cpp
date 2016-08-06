//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "drivers/hpet.hpp"

#include "acpica.hpp"
#include "logging.hpp"
#include "mmap.hpp"
#include "arch.hpp"

namespace {

ACPI_TABLE_HPET* hpet_table;
uint64_t* hpet_map;

// Offset of the registers inside the hpet memory
constexpr const size_t CAPABILITIES_REGISTER = 0x0;
constexpr const size_t GENERAL_CONFIG_REGISTER = 0x10 / 8;
constexpr const size_t MAIN_COUNTER = 0xF0 / 8;

constexpr const size_t GENERAL_CONFIG_ENABLE = 1 << 0;
constexpr const size_t GENERAL_CONFIG_LEGACY = 1 << 1;

uint64_t read_register(size_t reg){
    return hpet_map[reg];
}

void write_register(size_t reg, uint64_t value){
    hpet_map[reg] = value;
}

void set_register_bits(size_t reg, uint64_t bits){
    write_register(reg, read_register(reg) | bits);
}

void clear_register_bits(size_t reg, uint64_t bits){
    write_register(reg, read_register(reg) & ~bits);
}

} //End of anonymous namespace

bool hpet::install(){
    // Find the HPET table
    auto status = AcpiGetTable(ACPI_SIG_HPET, 0, reinterpret_cast<ACPI_TABLE_HEADER **>(&hpet_table));
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

        hpet_map = static_cast<uint64_t*>(mmap_phys(hpet_table->Address.Address, 1024)); //TODO Check the size

        // Disable interrupts: We should not be interrupted from this point on
        size_t rflags;
        arch::disable_hwint(rflags);

        // Disable before configuration
        clear_register_bits(GENERAL_CONFIG_REGISTER, GENERAL_CONFIG_ENABLE);

        // Clear the main counter
        write_register(MAIN_COUNTER, 0);

        // Enable interrupts again
        arch::enable_hwint(rflags);
    }
}
