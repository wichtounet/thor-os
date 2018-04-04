//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "drivers/hpet.hpp"

#include "conc/int_lock.hpp"

#include "acpica.hpp"
#include "virtual_debug.hpp"
#include "logging.hpp"
#include "mmap.hpp"
#include "kernel.hpp" // For suspend_kernel
#include "scheduler.hpp" // For async init
#include "timer.hpp"     // For setting the frequency

#include "drivers/pit.hpp" // For uninstalling it

// TODO Ideally, we should run in periodic mode
// However, I've not been able to make it work (no interrupt are generated)

namespace {

// Offset of the registers inside the hpet memory
constexpr const size_t CAPABILITIES_REGISTER = 0x0;
constexpr const size_t GENERAL_CONFIG_REGISTER = 0x10 / 8;
constexpr const size_t GENERAL_INTERRUPT_REGISTER = 0x20 / 8;
constexpr const size_t MAIN_COUNTER = 0xF0 / 8;

constexpr const size_t GENERAL_CONFIG_ENABLE = 1 << 0;
constexpr const size_t GENERAL_CONFIG_LEGACY = 1 << 1;

constexpr const size_t CAPABILITIES_LEGACY = 1 << 15;
constexpr const size_t CAPABILITIES_64 = 1 << 13;

constexpr const size_t TIMER_CONFIG_ENABLE = 1 << 2;
constexpr const size_t TIMER_CONFIG_PERIODIC = 1 << 3;
constexpr const size_t TIMER_CONFIG_PERIODIC_CAP = 1 << 5;
constexpr const size_t TIMER_CONFIG_64 = 1 << 5;

constexpr const size_t TIMER_FREQUENCY_GOAL = 10000; // 1 tick every 100 microseconds

ACPI_TABLE_HPET* hpet_table;
volatile uint64_t* hpet_map;
volatile uint64_t comparator_update;

uint64_t timer_configuration_reg(uint64_t n){
    return (0x100 + 0x20 * n) / 8;
}

uint64_t timer_comparator_reg(uint64_t n){
    return (0x108 + 0x20 * n) / 8;
}

uint64_t timer_interrupt_reg(uint64_t n){
    return (0x110 + 0x20 * n) / 8;
}

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

void timer_handler(interrupt::syscall_regs*, void*){
    // Clears Tn_INT_STS
    set_register_bits(GENERAL_INTERRUPT_REGISTER, 1 << 0);

    // Sets the next event to fire an IRQ
    write_register(timer_comparator_reg(0), read_register(MAIN_COUNTER) + comparator_update);

    timer::tick();
}

} //End of anonymous namespace

void hpet::init(){
    // HPET needs ACPI
    scheduler::queue_async_init_task(hpet::late_install);
}

bool hpet::install(){
    // Find the HPET table
    auto status = AcpiGetTable(ACPI_SIG_HPET, 0, reinterpret_cast<ACPI_TABLE_HEADER **>(&hpet_table));
    if (ACPI_FAILURE(status)){
        return false;
    }

    logging::logf(logging::log_level::TRACE, "hpet: Found ACPI HPET table\n");
    logging::logf(logging::log_level::TRACE, "hpet: HPET Address: %h\n", hpet_table->Address.Address);

    hpet_map = static_cast<volatile uint64_t*>(mmap_phys(hpet_table->Address.Address, 1024)); //TODO Check the size

    auto capabilities = read_register(CAPABILITIES_REGISTER);

    logging::logf(logging::log_level::TRACE, "hpet: HPET Capabilities: %B\n", capabilities);

    if(!(capabilities & CAPABILITIES_LEGACY)){
        logging::logf(logging::log_level::TRACE, "hpet: HPET is not able to handle legacy replacement mode\n");
        return false;
    }

    if(!(capabilities & CAPABILITIES_64)){
         logging::logf(logging::log_level::TRACE, "hpet: HPET is not able to handle 64-bit counter\n");
        return false;
    }

    if(!(read_register(timer_configuration_reg(0)) & TIMER_CONFIG_64)){
         logging::logf(logging::log_level::TRACE, "hpet: HPET Timer #0 is not 64-bit\n");
        return false;
    }

    return true;
}

void hpet::late_install(){
    if(hpet::install()){
        logging::logf(logging::log_level::TRACE, "hpet: Late install suceeded\n");

        // Disable interrupts: We should not be interrupted from this point on
        direct_int_lock lock;

        // Disable before configuration
        clear_register_bits(GENERAL_CONFIG_REGISTER, GENERAL_CONFIG_ENABLE);

        // Get the frequency of the main counter

        auto hpet_period = read_register(CAPABILITIES_REGISTER) >> 32;
        auto hpet_frequency = 1000000000000000 / hpet_period;

        uint64_t current_frequency;
        if(hpet_frequency >= TIMER_FREQUENCY_GOAL){
            current_frequency = TIMER_FREQUENCY_GOAL;
        } else {
            current_frequency = hpet_frequency;
        }

        comparator_update = hpet_frequency / current_frequency;

        logging::logf(logging::log_level::TRACE, "hpet: period %u\n", hpet_period);
        logging::logf(logging::log_level::TRACE, "hpet: frequency %uHz\n", hpet_frequency);
        logging::logf(logging::log_level::TRACE, "hpet: IRQ frequency %uHz\n", current_frequency);
        logging::logf(logging::log_level::TRACE, "hpet: Comparator update %u\n", comparator_update);

        // Update the current frequency (this will update the sleeping task as well)
        timer::timer_frequency(current_frequency);

        // Give information about the counter
        timer::counter_fun(hpet::counter);
        timer::counter_frequency(hpet_frequency);

        // Uninstall the PIT driver
        pit::remove();

        // Register the IRQ handler
        if(!interrupt::register_irq_handler(0, timer_handler, nullptr)){
            logging::logf(logging::log_level::ERROR, "Unable to register HPET IRQ handler 0\n");
            // TODO At this point, we should reinstall the PIT driver
            suspend_kernel();
            return;
        }

        // Clear the main counter
        write_register(MAIN_COUNTER, 0);

        // Initialize timer #0
        clear_register_bits(timer_configuration_reg(0), TIMER_CONFIG_PERIODIC);
        set_register_bits(timer_configuration_reg(0), TIMER_CONFIG_ENABLE);
        write_register(timer_comparator_reg(0), comparator_update);

        // Enable HPET in legacy mode
        set_register_bits(GENERAL_CONFIG_REGISTER, GENERAL_CONFIG_LEGACY | GENERAL_CONFIG_ENABLE);
    }
}

uint64_t hpet::counter(){
    return read_register(MAIN_COUNTER);
}
