//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "network.hpp"
#include "pci.hpp"
#include "rtl8139.hpp"
#include "physical_allocator.hpp"
#include "scheduler.hpp"

#include "fs/sysfs.hpp"

namespace {

std::vector<network::interface_descriptor> interfaces;

} //end of anonymous namespace

void network::init(){
    size_t index = 0;

    for(size_t i = 0; i < pci::number_of_devices(); ++i){
        auto& pci_device = pci::device(i);

        if(pci_device.class_type == pci::device_class_type::NETWORK){
            interfaces.emplace_back();

            auto& interface = interfaces.back();

            interface.name = std::string("net") + std::to_string(index);
            interface.pci_device = i;
            interface.enabled = false;
            interface.driver = "";
            interface.driver_data = nullptr;

            if(pci_device.vendor_id == 0x10EC && pci_device.device_id == 0x8139){
                interface.enabled = true;
                interface.driver = "rtl8139";

                rtl8139::init_driver(interface, pci_device);
            }

            std::string path = "/net/" + interface.name;

            sysfs::set_constant_value("/sys/", path + "/name", interface.name);
            sysfs::set_constant_value("/sys/", path + "/driver", interface.driver);
            sysfs::set_constant_value("/sys/", path + "/enabled", interface.enabled ? "true" : "false");
            sysfs::set_constant_value("/sys/", path + "/pci_device", std::to_string(i));
            sysfs::set_constant_value("/sys/", path + "/mac", std::to_string(interface.mac_address));

            ++index;
        }
    }
}

void network::finalize(){
    for(auto& interface : interfaces){
        // if the interface has a driver
        if(interface.enabled){
            auto* user_stack = new char[scheduler::user_stack_size];
            auto* kernel_stack = new char[scheduler::kernel_stack_size];

            //TODO Create the system process
        }
    }
}

size_t network::number_of_interfaces(){
    return interfaces.size();
}

network::interface_descriptor& network::interface(size_t index){
    return interfaces[index];
}
