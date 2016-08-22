//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ethernet_layer.hpp"
#include "icmp_layer.hpp"
#include "logging.hpp"
#include "arp_cache.hpp"
#include "arp_layer.hpp"
#include "kernel_utils.hpp"

void network::icmp::ping(network::interface_descriptor& interface, network::ip::address target_ip){
    logging::logf(logging::log_level::TRACE, "icmp: Ping %u.%u.%u.%u \n",
        uint64_t(target_ip(0)), uint64_t(target_ip(1)), uint64_t(target_ip(2)), uint64_t(target_ip(3)));

    auto target_mac = network::arp::get_mac_force(interface, target_ip);

    logging::logf(logging::log_level::TRACE, "icmp: Target MAC Address: %h\n", target_mac);

    //TODO
}
