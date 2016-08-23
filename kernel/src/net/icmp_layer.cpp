//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "net/ethernet_layer.hpp"
#include "net/icmp_layer.hpp"
#include "net/arp_cache.hpp"
#include "net/ip_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"

namespace {

uint16_t echo_sequence = 0;

void compute_checksum(network::icmp::header* header, size_t payload_size){
    header->checksum = 0;

    uint32_t sum = 0;
    std::for_each(reinterpret_cast<uint16_t*>(header), reinterpret_cast<uint16_t*>(header) + payload_size * 2, [&sum](uint16_t value){
        sum += value;
    });

    uint32_t value = sum & 0xFF;
    uint32_t carry = sum - value;

    while(carry){
        value += carry;
        auto sub = value & 0xFF;
        carry = value - sub;
        value = sub;
    }

    header->checksum = ~value;
}

} // end of anonymous namespace

void network::icmp::decode(network::interface_descriptor& interface, network::ethernet::packet& packet){
    header* icmp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "icmp: Start ICMP packet handling\n");

    //TODO
}

void network::icmp::ping(network::interface_descriptor& interface, network::ip::address target_ip){
    logging::logf(logging::log_level::TRACE, "icmp: Ping %u.%u.%u.%u \n",
        uint64_t(target_ip(0)), uint64_t(target_ip(1)), uint64_t(target_ip(2)), uint64_t(target_ip(3)));

    auto target_mac = network::arp::get_mac_force(interface, target_ip);

    logging::logf(logging::log_level::TRACE, "icmp: Target MAC Address: %h\n", target_mac);

    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(interface, sizeof(header), target_ip, 0x01);

    auto* icmp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    icmp_header->type = static_cast<uint8_t>(type::ECHO_REQUEST);
    icmp_header->code = 0;
    icmp_header->rest = 0;

    auto* command_header = reinterpret_cast<echo_request_header*>(&icmp_header->rest);

    command_header->identifier = 0x666;
    command_header->sequence = echo_sequence++;

    compute_checksum(icmp_header, 0);

    // Give the packet to the IP layer for finalization
    network::ip::finalize_packet(interface, packet);
}
