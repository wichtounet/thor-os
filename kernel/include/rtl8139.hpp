//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef RTL_8139_H
#define RTL_8139_H

#include<types.hpp>

#include "net/network.hpp"
#include "pci.hpp"

namespace rtl8139 {

void init_driver(network::interface_descriptor& interface, pci::device_descriptor& pci_device);

} //end of namespace rtl8139

#endif
