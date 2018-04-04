//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef RTL_8139_H
#define RTL_8139_H

#include <types.hpp>

#include "net/network.hpp"

#include "drivers/pci.hpp"

namespace rtl8139 {

void init_driver(network::interface_descriptor& interface, pci::device_descriptor& pci_device);
void finalize_driver(network::interface_descriptor& interface);

} //end of namespace rtl8139

#endif
