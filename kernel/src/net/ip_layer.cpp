//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "net/ip_layer.hpp"
#include "net/icmp_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"

inline network::ip::address network::ip::ip32_to_ip(uint32_t raw){
    return {switch_endian_32(raw)};
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
