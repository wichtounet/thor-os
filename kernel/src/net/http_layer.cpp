//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/http_layer.hpp"
#include "net/tcp_layer.hpp"

#include "kernel_utils.hpp"

namespace {

} //end of anonymous namespace

void network::http::decode(network::interface_descriptor& /*interface*/, network::ethernet::packet& packet) {
    packet.tag(3, packet.index);

    logging::logf(logging::log_level::TRACE, "http: Start HTTP packet handling\n");

    //TODO ?

    //TODO network::propagate_packet(packet, network::socket_protocol::HTTP);
}

std::expected<network::ethernet::packet> network::http::user_prepare_packet(char* buffer, network::interface_descriptor& interface, const packet_descriptor* descriptor) {
    // Ask the TCP layer to craft a packet
    //network::tcp::packet_descriptor desc{descriptor->target_ip, descriptor->source_port, 80, descriptor->payload_size};
    //auto packet = network::tcp::user_prepare_packet(buffer, interface, &desc);

    //if (packet) {
        //packet.tag(3, packet.index);
    //}

    //return packet;
}

std::expected<void> network::http::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p) {
    // Give the packet to the TCP layer for finalization
    //TODO return network::tcp::finalize_packet(interface, p);
}
