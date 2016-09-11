//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>

#include "net/dns_layer.hpp"
#include "net/udp_layer.hpp"

#include "kernel_utils.hpp"

namespace {

using flag_qr     = std::bit_field<uint16_t, uint8_t, 0, 1>;
using flag_opcode = std::bit_field<uint16_t, uint8_t, 1, 4>;
using flag_aa     = std::bit_field<uint16_t, uint8_t, 5, 1>;
using flag_tc     = std::bit_field<uint16_t, uint8_t, 6, 1>;
using flag_rd     = std::bit_field<uint16_t, uint8_t, 7, 1>;
using flag_ra     = std::bit_field<uint16_t, uint8_t, 8, 1>;
using flag_zeroes = std::bit_field<uint16_t, uint8_t, 9, 3>;
using flag_rcode  = std::bit_field<uint16_t, uint8_t, 12, 4>;

void prepare_packet_query(network::ethernet::packet& packet, uint16_t identification){
    packet.tag(3, packet.index);

    // Set the DNS header

    auto* dns_header = reinterpret_cast<network::dns::header*>(packet.payload + packet.index);

    // Set the identification
    dns_header->identification = switch_endian_16(identification);

    // There is one question, nothing else
    dns_header->questions      = switch_endian_16(1);
    dns_header->answers        = switch_endian_16(0);
    dns_header->authority_rrs  = switch_endian_16(0);
    dns_header->additional_rrs = switch_endian_16(0);

    // Set all the flags
    flag_qr(&dns_header->flags) = 0;     // This is a query
    flag_opcode(&dns_header->flags) = 0; // This is a standard query
    flag_aa(&dns_header->flags) = 0;     // This is a query (field not used)
    flag_tc(&dns_header->flags) = 0;     // The question is not truncated
    flag_rd(&dns_header->flags) = 0;     // No need for recursion
    flag_ra(&dns_header->flags) = 0;     // This is a query (field not used)
    flag_zeroes(&dns_header->flags) = 0; // Always zero
    flag_rcode(&dns_header->flags) = 0;  // This is a query (field not used)

    packet.index += sizeof(network::dns::header);
}

} //end of anonymous namespace

void network::dns::decode(network::interface_descriptor& /*interface*/, network::ethernet::packet& packet){
    packet.tag(3, packet.index);

    auto* dns_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "dns: Start DNS packet handling\n");

    auto identification = switch_endian_16(dns_header->identification);
    auto questions      = switch_endian_16(dns_header->questions);
    auto answers        = switch_endian_16(dns_header->answers);
    auto authority_rrs  = switch_endian_16(dns_header->authority_rrs);
    auto additional_rrs = switch_endian_16(dns_header->additional_rrs);

    logging::logf(logging::log_level::TRACE, "dns: Identification %h \n", size_t(identification));
    logging::logf(logging::log_level::TRACE, "dns: Answers %u \n", size_t(answers));
    logging::logf(logging::log_level::TRACE, "dns: Questions %u \n", size_t(questions));
    logging::logf(logging::log_level::TRACE, "dns: Authorithy RRs %u \n", size_t(authority_rrs));
    logging::logf(logging::log_level::TRACE, "dns: Additional RRs %u \n", size_t(additional_rrs));

    auto flags = dns_header->flags;

    if(*flag_qr(&dns_header->flags)){
        logging::logf(logging::log_level::TRACE, "dns: Query\n");
    } else {
        auto response_code = *flag_opcode(&dns_header->flags);

        if(response_code == 0x0){
            logging::logf(logging::log_level::TRACE, "dns: Response OK\n");
        } else if(response_code == 0x1){
            logging::logf(logging::log_level::TRACE, "dns: Format Error\n");
        } else if(response_code == 0x2){
            logging::logf(logging::log_level::TRACE, "dns: Server Failure\n");
        } else if(response_code == 0x3){
            logging::logf(logging::log_level::TRACE, "dns: Name Error\n");
        } else if(response_code == 0x4){
            logging::logf(logging::log_level::TRACE, "dns: Not Implemented\n");
        } else if(response_code == 0x5){
            logging::logf(logging::log_level::TRACE, "dns: Refused\n");
        }
    }

    //TODO
}

std::expected<network::ethernet::packet> network::dns::prepare_packet_query(network::interface_descriptor& interface, network::ip::address target_ip, uint16_t source_port, uint16_t identification, size_t payload_size){
    // Ask the UDP layer to craft a packet
    auto packet = network::udp::prepare_packet(interface, target_ip, source_port, 53, sizeof(header) + payload_size);

    if(packet){
        ::prepare_packet_query(*packet, identification);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::dns::prepare_packet_query(char* buffer, network::interface_descriptor& interface, network::ip::address target_ip, uint16_t source_port, uint16_t identification, size_t payload_size){
    // Ask the UDP layer to craft a packet
    auto packet = network::udp::prepare_packet(buffer, interface, target_ip, source_port, 53, sizeof(header) + payload_size);

    if(packet){
        ::prepare_packet_query(*packet, identification);
    }

    return packet;
}

void network::dns::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p){
    p.index -= sizeof(header);

    // Give the packet to the UDP layer for finalization
    network::udp::finalize_packet(interface, p);
}
