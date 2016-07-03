//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PCI_H
#define PCI_H

#include<types.hpp>

namespace pci {

enum class device_class_type : uint8_t {
    OLD = 0x0,
    MASS_STORAGE = 0x1,
    NETWORK = 0x2,
    DISPLAY = 0x3,
    MULTIMEDIA = 0x4,
    MEMORY = 0x5,
    BRIDGE = 0x6,
    SIMPLE_COM = 0x7,
    BASE_SYSTEM = 0x8,
    INPUT = 0x9,
    DOCKING_STATION = 0xA,
    PROCESSOR = 0xB,
    SERIAL = 0xC,
    WIRELESS = 0xD,
    SMART_IO = 0xE,
    SATELLITE = 0xF,
    ENCRYPTION = 0x10,
    DATA_ACQUISITION = 0x11,
    RESERVED = 0x12,
    UNKNOWN = 0x13
};

struct device_descriptor {
    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t sub_class;
    device_class_type class_type;
};

void detect_devices();
size_t number_of_devices();
device_descriptor& device(size_t index);

} //end of namespace pci

#endif
