//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/alpha.hpp"
#include "net/ethernet_layer.hpp"
#include "net/icmp_layer.hpp"
#include "net/arp_cache.hpp"
#include "net/ip_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"
#include "scheduler.hpp"

#include "tlib/errors.hpp"

namespace {

size_t echo_sequence = 1;

} // end of anonymous namespace

void network::alpha(){
    auto& interface = network::interface(0);

    auto target_ip = network::ip::make_address(10,0,2,2);

    logging::logf(logging::log_level::TRACE, "icmp: Ping %u.%u.%u.%u \n",
        uint64_t(target_ip(0)), uint64_t(target_ip(1)), uint64_t(target_ip(2)), uint64_t(target_ip(3)));

    auto target_mac = network::arp::get_mac_force(interface, target_ip);

    if(!target_mac){
        logging::logf(logging::log_level::TRACE, "icmp: Failed to get MAC Address from IP\n");
        return;
    }

    logging::logf(logging::log_level::TRACE, "icmp: Target MAC Address: %h\n", *target_mac);

    // Ask the ICMP layer to craft a packet
    auto packet = network::icmp::prepare_packet(interface, target_ip, 0, network::icmp::type::ECHO_REQUEST, 0);

    if(packet){
        // Set the Command header

        auto* command_header = reinterpret_cast<network::icmp::echo_request_header*>(packet->payload + packet->index);

        command_header->identifier = 0x666;
        command_header->sequence = echo_sequence++;

        // Send the packet back to ICMP
        network::icmp::finalize_packet(interface, *packet);
    }
}
