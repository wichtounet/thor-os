//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "arp_layer.hpp"
#include "logging.hpp"
#include "kernel_utils.hpp"

namespace {

//TODO

} //end of anonymous namespace

void network::arp::decode(network::ethernet::packet& packet){
    header* arp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "arp: Start ARP packet handling\n");

    auto hw_type = switch_endian_16(arp_header->hw_type);
    auto protocol_type = switch_endian_16(arp_header->protocol_type);

    if(hw_type != 0x1){
        logging::logf(logging::log_level::ERROR, "arp: Unhandled hardware type %h\n", size_t(hw_type));
        return;
    }

    if(protocol_type != 0x800){
        logging::logf(logging::log_level::ERROR, "arp: Unhandled protocol type %h\n", size_t(protocol_type));
        return;
    }

    auto hw_len = arp_header->hw_len;
    auto protocol_len = arp_header->protocol_len;
    auto operation = switch_endian_16(arp_header->operation);

    logging::logf(logging::log_level::TRACE, "arp: Start ARP packet handling %h\n", size_t(hw_len));
    logging::logf(logging::log_level::TRACE, "arp: Start ARP packet handling %h\n", size_t(protocol_len));
    logging::logf(logging::log_level::TRACE, "arp: Start ARP packet handling %h\n", size_t(operation));
}
