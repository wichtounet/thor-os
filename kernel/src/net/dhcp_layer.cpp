//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>

#include "net/dhcp_layer.hpp"
#include "net/udp_layer.hpp"

#include "kernel_utils.hpp"

#include "tlib/errors.hpp"

namespace {

void prepare_packet(network::ethernet::packet& packet, uint16_t identification) {
    packet.tag(3, packet.index);

    // Set the DNS header

    auto* dhcp_header = reinterpret_cast<network::dns::header*>(packet.payload + packet.index);

    //TODO

    packet.index += sizeof(network::dns::header);
}

} //end of anonymous namespace

void network::dhcp::decode(network::interface_descriptor& /*interface*/, network::ethernet::packet& packet) {
    packet.tag(3, packet.index);

    auto* dhcp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    // Note: Propagate is handled by UDP connections
}

std::expected<network::ip::address> network::dhcp::request_ip(network::interface_descriptor& interface){

}
