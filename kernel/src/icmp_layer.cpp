//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "icmp_layer.hpp"
#include "logging.hpp"

void network::icmp::ping(network::interface_descriptor& interface, network::ip::address target){
    logging::logf(logging::log_level::TRACE, "icmp: Ping %u.%u.%u.%u \n",
        uint64_t(target(0)), uint64_t(target(1)), uint64_t(target(2)), uint64_t(target(3)));

    //TODO
}
