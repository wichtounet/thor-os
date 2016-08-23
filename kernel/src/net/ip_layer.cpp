//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "net/ip_layer.hpp"
#include "net/ethernet_layer.hpp"
#include "net/icmp_layer.hpp"
#include "net/arp_cache.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"

namespace {

void compute_checksum(network::ip::header* header){
    auto ihl = header->version_ihl & 0xF;

    header->header_checksum = 0;

    uint32_t sum = 0;
    std::for_each(reinterpret_cast<uint16_t*>(header),reinterpret_cast<uint16_t*>(header) + ihl * 2, [&sum](uint16_t value){
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

    header->header_checksum = ~value;
}

} // end of anonymous namespace

network::ip::address network::ip::ip32_to_ip(uint32_t raw){
    return {switch_endian_32(raw)};
}

uint32_t network::ip::ip_to_ip32(address ip){
    return switch_endian_32(ip.raw_address);
}

void network::ip::decode(network::interface_descriptor& interface, network::ethernet::packet& packet){
    header* ip_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "ip: Start IPv4 packet handling\n");

    auto version = ip_header->version_ihl >> 4;

    if(version != 4){
        logging::logf(logging::log_level::ERROR, "ip: IPv6 Packet received instead of IPv4\n");

        return;
    }

    auto ihl = ip_header->version_ihl & 0xF;
    auto length = switch_endian_16(ip_header->total_len);
    auto data_length = length - ihl * 4;

    logging::logf(logging::log_level::TRACE, "ip: Data Length: %u\n", size_t(data_length));
    logging::logf(logging::log_level::TRACE, "ip: Time To Live: %u\n", size_t(ip_header->ttl));

    auto source = ip32_to_ip(ip_header->source_ip);
    auto target = ip32_to_ip(ip_header->target_ip);

    logging::logf(logging::log_level::TRACE, "ip: Source Protocol Address %u.%u.%u.%u \n",
        uint64_t(source(0)), uint64_t(source(1)), uint64_t(source(2)), uint64_t(source(3)));
    logging::logf(logging::log_level::TRACE, "ip: Target Protocol Address %u.%u.%u.%u \n",
        uint64_t(target(0)), uint64_t(target(1)), uint64_t(target(2)), uint64_t(target(3)));

    auto protocol = ip_header->protocol;

    packet.index += sizeof(header);

    if(protocol == 0x01){
        network::icmp::decode(interface, packet);
    } else if(protocol == 0x06){
        logging::logf(logging::log_level::ERROR, "ip: TCP packet detected (unsupported)\n");
    } else if(protocol == 0x11){
        logging::logf(logging::log_level::ERROR, "ip: UDP packet detected (unsupported)\n");
    } else {
        logging::logf(logging::log_level::ERROR, "ip: Packet of unknown protocol detected (%h)\n", size_t(protocol));
    }
}

network::ethernet::packet network::ip::prepare_packet(network::interface_descriptor& interface, size_t size, address& target_ip, size_t protocol){
    auto target_mac = network::arp::get_mac_force(interface, target_ip);

    // Ask the ethernet layer to craft a packet
    auto packet = network::ethernet::prepare_packet(interface, size + sizeof(header), target_mac, ethernet::ether_type::IPV4);

    auto* header = reinterpret_cast<network::ip::header*>(packet.payload + packet.index);

    header->version_ihl = (4 << 4) + 5;
    header->dscp_ecn = 0;
    header->total_len = size + sizeof(header);
    header->identification = 0;
    header->flags_offset = 1U << 14;
    header->ttl = 255;
    header->protocol = protocol;
    header->source_ip = ip_to_ip32(interface.ip_address);
    header->target_ip = ip_to_ip32(target_ip);

    compute_checksum(header);

    packet.index += sizeof(network::ip::header);

    return packet;
}

void network::ip::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p){
    // Send the packet to the ethernet layer
    network::ethernet::finalize_packet(interface, p);
}
