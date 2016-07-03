//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "pci.hpp"
#include "kernel_utils.hpp"
#include "logging.hpp"

#include "fs/sysfs.hpp"

namespace {

std::vector<pci::device_descriptor> devices;

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

 uint32_t pci_config_read_dword (uint8_t bus, uint8_t device, uint8_t function, uint8_t offset){
    uint32_t address =
            static_cast<uint32_t>(1 << 31)  //enabled
        | (uint32_t(bus) << 16)  //bus number
        | (uint32_t(device) << 11)  //device number
        | (uint32_t(function) << 8) //function number
        | ((uint32_t(offset) ) & 0xfc); //Register number

    out_dword(PCI_CONFIG_ADDRESS, address);

    return in_dword(PCI_CONFIG_DATA);
 }

 uint16_t get_vendor_id(uint8_t bus, uint8_t device, uint8_t function){
     // read "device id | vendor id"
     auto large = pci_config_read_dword(bus, device, function, 0);
     // extract vendor id
     return large;
 }

 uint16_t get_device_id(uint8_t bus, uint8_t device, uint8_t function){
     // read "device id | vendor id"
     auto large = pci_config_read_dword(bus, device, function, 0);
     // extract device id
     return large >> 16;
 }

 uint8_t get_class_code(uint8_t bus, uint8_t device, uint8_t function){
     // read "class code | subclass | prog if | revision id"
     auto large = pci_config_read_dword(bus, device, function, 8);
     // extract class code only
     return large >> 24 & 0xFF;
 }

 uint8_t get_subclass(uint8_t bus, uint8_t device, uint8_t function){
     // read "class code | subclass | prog if | revision id"
     auto large = pci_config_read_dword(bus, device, function, 8);
     // extract subclass only
     return large >> 16 & 0xFF;
 }

 uint8_t get_header_type(uint8_t bus, uint8_t device, uint8_t function){
     // read "BIST | header type | latency timer | cache line size"
     auto large = pci_config_read_dword(bus, device, function, 12);
     // extract header type only
     return large >> 16 & 0xFF;
 }

 void check_function(uint8_t bus, uint8_t device, uint8_t function){
     auto vendor_id = get_vendor_id(bus, device, function);
     if(vendor_id == 0xFFFF) {
         return;
     }

     auto device_id = get_device_id(bus, device, function);
     auto class_code = get_class_code(bus, device, function);
     auto sub_class = get_subclass(bus, device, function);

     logging::logf(logging::log_level::DEBUG, "Found device pci:%u:%u:%u (vendor:%u class:%u subclass:%u) \n",
         uint64_t(bus), uint64_t(device), uint64_t(function), uint64_t(vendor_id), uint64_t(class_code), uint64_t(sub_class));

     devices.emplace_back();

     pci::device_descriptor& device_desc = devices.back();

     device_desc.bus = bus;
     device_desc.device = device;
     device_desc.function = function;
     device_desc.vendor_id = vendor_id;
     device_desc.device_id = device_id;
     device_desc.class_code = class_code;
     device_desc.sub_class = sub_class;

     if(class_code < static_cast<uint8_t>(pci::device_class_type::RESERVED)){
        device_desc.class_type = static_cast<pci::device_class_type>(class_code);
     } else if(class_code < static_cast<uint8_t>(pci::device_class_type::UNKNOWN)){
        device_desc.class_type = pci::device_class_type::RESERVED;
     } else {
        device_desc.class_type = pci::device_class_type::UNKNOWN;
     }

     std::string path = "/pci/pci:" + std::to_string(bus) + ':' + std::to_string(device) + ':' + std::to_string(function);

     sysfs::set_constant_value("/sys/", path + "/vendor", std::to_string(vendor_id));
     sysfs::set_constant_value("/sys/", path + "/device", std::to_string(device_id));
     sysfs::set_constant_value("/sys/", path + "/class", std::to_string(class_code));
     sysfs::set_constant_value("/sys/", path + "/subclass", std::to_string(sub_class));
 }

 void check_device(uint8_t bus, uint8_t device) {
     check_function(bus, device, 0);

     auto header_type = get_header_type(bus, device, 0);
     if((header_type & 0x80) != 0){
         for(uint8_t function = 1; function < 8; ++function){
             check_function(bus, device, function);
         }
     }
 }

void brute_force_check_all_buses(){
    for(uint16_t bus = 0; bus < 256; ++bus) {
        for(uint8_t device = 0; device < 32; ++device) {
            check_device(bus, device);
        }
    }
}

} //end of anonymous namespace

void pci::detect_devices(){
    brute_force_check_all_buses();
}

size_t pci::number_of_devices(){
    return devices.size();
}

pci::device_descriptor& pci::device(size_t index){
    return devices[index];
}
