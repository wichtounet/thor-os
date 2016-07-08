//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NETWORK_H
#define NETWORK_H

#include <types.hpp>
#include <string.hpp>

namespace network {

struct interface_descriptor {
    bool enabled;
    std::string name;
    std::string driver;
    size_t pci_device;
    size_t mac_address;
    void* driver_data;
};

void init();

size_t number_of_interfaces();

interface_descriptor& interface(size_t index);

} // end of network namespace

#endif
