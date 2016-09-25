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

void prepare_packet(network::ethernet::packet& packet, network::interface_descriptor& interface) {
    packet.tag(3, packet.index);

    // Set the DHCP header

    auto* dhcp_header = reinterpret_cast<network::dhcp::header*>(packet.payload + packet.index);

    dhcp_header->htype = 1; // Ethernet hardware
    dhcp_header->hlen  = 6; // Ethernet Mac Address
    dhcp_header->hops  = 0; // No relays
    dhcp_header->secs  = 0; // Not used for us

    dhcp_header->client_ip = 0; // Cleared by default
    dhcp_header->your_ip   = 0; // Cleared by default
    dhcp_header->server_ip = 0; // Cleared by default
    dhcp_header->gw_ip     = 0; // Cleared by default

    // Set the MAC address

    char mac[6];
    network::ethernet::mac64_to_mac6(interface.mac_address, mac);

    std::copy_n(mac, 6, dhcp_header->client_haddr);
    std::fill_n(dhcp_header->client_haddr + 6, 10, 0);

    // Clear the legacy BOOTP fields

    std::fill_n(dhcp_header->server_name, 64, 0);
    std::fill_n(dhcp_header->boot_filename, 128, 0);

    packet.index += sizeof(network::dhcp::header);
}

} //end of anonymous namespace

void network::dhcp::decode(network::interface_descriptor& /*interface*/, network::ethernet::packet& packet) {
    packet.tag(3, packet.index);

    logging::logf(logging::log_level::TRACE, "dhcp: Start DHCP packet handling\n");

    auto* dhcp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "dhcp: Identification: %u\n", size_t(dhcp_header->xid));

    // Note: Propagate is handled by UDP connections
}

std::expected<network::ip::address> network::dhcp::request_ip(network::interface_descriptor& interface){
    // 1. Send DHCP Discovery

    {
        // Ask the UDP layer to craft a packet
        auto payload_size = sizeof(network::dhcp::header) + 4 + 3 + 5 + 4;
        network::udp::kernel_packet_descriptor udp_desc{payload_size, 68, 67, network::ip::make_address(255, 255, 255, 255)};

        auto packet = network::udp::kernel_prepare_packet(interface, udp_desc);

        if(!packet){
            return std::make_unexpected<network::ip::address>(packet.error());
        }

        ::prepare_packet(*packet, interface);

        auto* dhcp_header = reinterpret_cast<network::dhcp::header*>(packet->payload + packet->tag(3));

        dhcp_header->op    = 1;          // This is a request
        dhcp_header->xid   = 0x66666666; // Our identification
        dhcp_header->flags = 0x8000;     // We don't know our IP address for now

        auto* options = packet->payload + packet->tag(3) + sizeof(network::dhcp::header);

        // Option 0: Magic cookie
        options[0] = 99;
        options[1] = 130;
        options[2] = 83;
        options[3] = 99;

        // Option 1: DHCP Discover
        options[4] = 53; // DHCP Message Type
        options[5] = 1;  // Length 1
        options[6] = 1;  // DHCP Discover

        // Option 2: Request Gateway / DNS / Mask
        options[7]  = 55; // Parameter Request List
        options[8]  = 3;  // Length 3
        options[9]  = 1;  // Subnet mask
        options[10] = 3;  // Router (Gateway)
        options[11] = 6;  // DNS Server

        // Pad 3 bytes
        options[12] = 0;
        options[13] = 0;
        options[14] = 0;

        // End of options
        options[15] = 255;

        // Finalize the UDP packet
        auto status = network::udp::finalize_packet(interface, *packet);
        if(!status){
            return std::make_unexpected<network::ip::address>(status.error());
        }
    }

    // 2. Receive DHCP Offer

    {

    }

    // 3. Send DHCP Request

    {

    }

    // 4. Receive DHCP Acknowledge

    {

    }
}
