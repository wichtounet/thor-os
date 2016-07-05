//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "ethernet_layer.hpp"
#include "logging.hpp"
#include "kernel_utils.hpp"

namespace {

network::ethernet::ether_type decode_ether_type(network::ethernet::header* header){
    auto type = switch_endian_16(header->type);

    if(type == 0x800){
        return network::ethernet::ether_type::IPV4;
    } else if(type == 0x86DD){
        return network::ethernet::ether_type::IPV6;
    } else if(type == 0x806){
        return network::ethernet::ether_type::ARP;
    } else {
        return network::ethernet::ether_type::UNKNOWN;
    }
}

} //end of anonymous namespace

void network::ethernet::decode(packet& packet){
    header* ether_header = reinterpret_cast<header*>(packet.payload);

    // Filter out non-ethernet II frames
    if(ether_header->type < 1536){
        logging::logf(logging::log_level::ERROR, "ethernet: error only ethernet frame type II is supported\n");
        return;
    }

    size_t source_mac = 0;
    size_t target_mac = 0;

    for(size_t i = 0; i < 6; ++i){
        source_mac |= uint64_t(ether_header->source.mac[i]) << ((5 - i) * 8);
        target_mac |= uint64_t(ether_header->target.mac[i]) << ((5 - i) * 8);
    }

    logging::logf(logging::log_level::TRACE, "ethernet: Source MAC Address %h \n", source_mac);
    logging::logf(logging::log_level::TRACE, "ethernet: Destination MAC Address %h \n", target_mac);

    packet.type = decode_ether_type(ether_header);

    switch(packet.type){
        case ether_type::IPV4:
            logging::logf(logging::log_level::TRACE, "ethernet: IPV4 Packet (unsupported)\n");
            break;
        case ether_type::IPV6:
            logging::logf(logging::log_level::TRACE, "ethernet: IPV6 Packet (unsupported)\n");
            break;
        case ether_type::ARP:
            logging::logf(logging::log_level::TRACE, "ethernet: ARP Packet (unsupported)\n");
            break;
        case ether_type::UNKNOWN:
            logging::logf(logging::log_level::TRACE, "ethernet: Unhandled Packet Type: %u\n", uint64_t(switch_endian_16(ether_header->type)));
            break;
    }
}
