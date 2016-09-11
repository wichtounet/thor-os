//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/dns_layer.hpp"
#include "net/udp_layer.hpp"

#include "kernel_utils.hpp"

namespace {

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
    dns_header->flags.qr     = 0; // This is a query
    dns_header->flags.opcode = 0; // This is a standard query
    dns_header->flags.aa     = 0; // This is a query (field not used)
    dns_header->flags.tc     = 0; // The question is not truncated
    dns_header->flags.rd     = 0; // No need for recursion
    dns_header->flags.ra     = 0; // This is a query (field not used)
    dns_header->flags.zeroes = 0; // Always zero
    dns_header->flags.rcode  = 0; // This is a query (field not used)

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
    logging::logf(logging::log_level::TRACE, "dns: Answers %h \n", size_t(answers));
    logging::logf(logging::log_level::TRACE, "dns: Questions %h \n", size_t(questions));
    logging::logf(logging::log_level::TRACE, "dns: Authorithy RRs %h \n", size_t(authority_rrs));
    logging::logf(logging::log_level::TRACE, "dns: Additional RRs %h \n", size_t(additional_rrs));

    auto flags = dns_header->flags;

    if(flags.qr){
        logging::logf(logging::log_level::TRACE, "dns: Query\n");
    } else {
        logging::logf(logging::log_level::TRACE, "dns: Response\n");
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
