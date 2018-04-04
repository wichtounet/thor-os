//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "drivers/rtc.hpp"
#include "kernel_utils.hpp"
#include "acpi.hpp"
#include "acpica.hpp"
#include "logging.hpp"

namespace {

#define CURRENT_YEAR        2013
#define cmos_address        0x70
#define cmos_data           0x71

int get_update_in_progress_flag(){
    out_byte(cmos_address, 0x0A);
    return (in_byte(cmos_data) & 0x80);
}

uint8_t get_RTC_register(int reg){
    out_byte(cmos_address, reg);
    return in_byte(cmos_data);
}

} //end of anonymous namespace

rtc::datetime rtc::all_data(){
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century = 0x0;

    uint8_t last_second;
    uint8_t last_minute;
    uint8_t last_hour;
    uint8_t last_day;
    uint8_t last_month;
    uint8_t last_year;
    uint8_t last_century;
    uint8_t registerB;

    int century_register = 0x0;

    if (acpi::initialized() && AcpiGbl_FADT.Header.Revision >= FADT2_REVISION_ID && AcpiGbl_FADT.Century){
        century_register = AcpiGbl_FADT.Century;
    }

    while (get_update_in_progress_flag()){};                // Make sure an update isn't in progress

    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);

    if(century_register){
        century = get_RTC_register(century_register);
    }

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;

        while (get_update_in_progress_flag()){};           // Make sure an update isn't in progress

        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);

        if(century_register){
            century = get_RTC_register(century_register);
        }
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
        (last_day != day) || (last_month != month) || (last_year != year) || (last_century != century));

    registerB = get_RTC_register(0x0B);

    // Convert BCD to binary values if necessary

    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        century = (century & 0x0F) + ((century / 16) * 10);
    }

    // Convert 12 hour clock to 24 hour clock if necessary

    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year

    uint16_t full_year;

    if(century_register){
        full_year = year + century * 100;
    } else {
        full_year = year + (CURRENT_YEAR / 100) * 100;
        if(full_year < CURRENT_YEAR){
            full_year += 100;
        }
    }

    return {full_year, month, day, hour, minute, second, 0, 0};
}
