//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DATETIME_H
#define DATETIME_H

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, rtc) {

struct datetime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t unused;
    uint64_t precise;
} __attribute__((packed)) ;

} // end of namespace tlib

#endif
