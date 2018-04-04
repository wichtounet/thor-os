//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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

uint8_t  read_config_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t read_config_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint32_t read_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

void write_config_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value);
void write_config_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);
void write_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

} //end of namespace pci

#endif
