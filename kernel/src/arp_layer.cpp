//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "arp_layer.hpp"
#include "ip_layer.hpp"
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

    // We only support ethernet hardware frames
    if(hw_type != 0x1){
        logging::logf(logging::log_level::ERROR, "arp: Unhandled hardware type %h\n", size_t(hw_type));
        return;
    }

    // We only support IPV4 protocol
    if(protocol_type != 0x800){
        logging::logf(logging::log_level::ERROR, "arp: Unhandled protocol type %h\n", size_t(protocol_type));
        return;
    }

    auto hw_len = arp_header->hw_len;
    auto protocol_len = arp_header->protocol_len;
    auto operation = switch_endian_16(arp_header->operation);

    // This should never happen
    if(hw_len != 0x6 || protocol_len != 0x4){
        logging::logf(logging::log_level::ERROR, "arp: Unable to process the given length hw:%h prot:%h\n", size_t(hw_len), size_t(protocol_len));
        return;
    }

    if (operation != 0x1 && operation != 0x2) {
        logging::logf(logging::log_level::TRACE, "arp: Unhandled operation %h\n", size_t(operation));
        return;
    }

    size_t source_hw = 0;
    size_t target_hw = 0;

    for(size_t i = 0; i < 3; ++i){
        source_hw |= uint64_t(switch_endian_16(arp_header->source_hw_addr[i])) << ((3 - i) * 16);
        target_hw |= uint64_t(switch_endian_16(arp_header->target_hw_addr[i])) << ((3 - i) * 16);
    }

    logging::logf(logging::log_level::TRACE, "arp: Source HW Address %h \n", source_hw);
    logging::logf(logging::log_level::TRACE, "arp: Target HW Address %h \n", target_hw);

    network::ip::address source_prot;
    network::ip::address target_prot;

    for(size_t i = 0; i < 2; ++i){
        auto source = switch_endian_16(arp_header->source_protocol_addr[i]);
        auto target = switch_endian_16(arp_header->target_protocol_addr[i]);

        source_prot.set_sub(2*i, source >> 8);
        source_prot.set_sub(2*i+1, source);

        target_prot.set_sub(2*i, target >> 8);
        target_prot.set_sub(2*i+1, target);
    }

    logging::logf(logging::log_level::TRACE, "arp: Source Protocol Address %u.%u.%u.%u \n",
        uint64_t(source_prot(0)), uint64_t(source_prot(1)), uint64_t(source_prot(2)), uint64_t(source_prot(3)));
    logging::logf(logging::log_level::TRACE, "arp: Target Protocol Address %u.%u.%u.%u \n",
        uint64_t(target_prot(0)), uint64_t(target_prot(1)), uint64_t(target_prot(2)), uint64_t(target_prot(3)));

    if(operation == 0x1){
        logging::logf(logging::log_level::TRACE, "arp: Handle Request\n");

        //TODO
    } else if(operation == 0x2){
        logging::logf(logging::log_level::TRACE, "arp: Handle Query\n");

        //TODO
    }
}
