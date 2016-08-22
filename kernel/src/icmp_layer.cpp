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

void network::icmp::ping(network::interface_descriptor& interface, network::ip::address target){
    logging::logf(logging::log_level::TRACE, "icmp: Ping %u.%u.%u.%u \n",
        uint64_t(target(0)), uint64_t(target(1)), uint64_t(target(2)), uint64_t(target(3)));

    if(!network::arp::is_ip_cached(target)){
        logging::logf(logging::log_level::TRACE, "icmp: IP not cached, generate request\n");

        // Ask the ethernet layer to craft a packet
        auto packet = network::ethernet::prepare_packet(interface, sizeof(network::arp::header), 0xFFFFFFFFFFFF, ethernet::ether_type::ARP);

        auto* arp_request_header = reinterpret_cast<network::arp::header*>(packet.payload + packet.index);

        arp_request_header->hw_type = switch_endian_16(0x1); // ethernet
        arp_request_header->protocol_type = switch_endian_16(0x800); // IPV4
        arp_request_header->hw_len = 0x6; // MAC Address
        arp_request_header->protocol_len = 0x4; // IP Address
        arp_request_header->operation = switch_endian_16(0x1); //ARP Request

        auto source_mac = interface.mac_address;

        for(size_t i = 0; i < 3; ++i){
            arp_request_header->target_hw_addr[i] = 0x0;
            arp_request_header->source_hw_addr[i] = switch_endian_16(uint16_t(source_mac >> ((2 - i) * 16)));
        }

        network::ip::address source = network::ip::make_address(192, 168, 20, 66);

        for(size_t i = 0; i < 2; ++i){
            arp_request_header->source_protocol_addr[i] = (uint16_t(source(2*i+1)) << 8) + source(2*i);
            arp_request_header->target_protocol_addr[i] = (uint16_t(target(2*i+1)) << 8) + target(2*i);
        }

        network::ethernet::finalize_packet(interface, packet);
    } else {
        logging::logf(logging::log_level::TRACE, "icmp: IP cached, MAC Address: %h\n", network::arp::get_mac(target));
    }

    //TODO
}
