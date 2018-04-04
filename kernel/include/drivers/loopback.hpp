//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef LOOPBACK_H
#define LOOPBACK_H

#include <types.hpp>

#include "net/network.hpp"

namespace loopback {

void init_driver(network::interface_descriptor& interface);
void finalize_driver(network::interface_descriptor& interface);

} //end of namespace loopback

#endif
