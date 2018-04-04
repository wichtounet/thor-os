//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/dhcp_layer.hpp"
#include "net/udp_layer.hpp"
#include "net/ethernet_layer.hpp"

#include "kernel_utils.hpp"

#include "tlib/errors.hpp"

namespace {

void prepare_packet(network::packet& packet, network::interface_descriptor& interface) {
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

network::dhcp::layer::layer(network::udp::layer* parent) : parent(parent) {
    parent->register_dhcp_layer(this);
}

void network::dhcp::layer::decode(network::interface_descriptor& /*interface*/, network::packet_p& packet) {
    packet->tag(3, packet->index);

    logging::logf(logging::log_level::TRACE, "dhcp: Start DHCP packet handling\n");

    auto* dhcp_header = reinterpret_cast<header*>(packet->payload + packet->index);

    logging::logf(logging::log_level::TRACE, "dhcp: Identification: %u\n", size_t(dhcp_header->xid));

    // Note: Propagate is handled by UDP connections

    if (listening.load()) {
        packets.push_back(packet);
        listen_queue.notify_one();
    }
}

std::expected<network::dhcp::dhcp_configuration> network::dhcp::layer::request_ip(network::interface_descriptor& interface) {
    logging::logf(logging::log_level::TRACE, "dhcp: Start discovery\n");

    listening = true;

    // 1. Send DHCP Discovery

    {
        // Ask the UDP layer to craft a packet
        auto payload_size = sizeof(network::dhcp::header) + 4 + 3 + 5 + 4;
        network::udp::kernel_packet_descriptor udp_desc{payload_size, 68, 67, network::ip::make_address(255, 255, 255, 255)};

        auto packet_e = parent->kernel_prepare_packet(interface, udp_desc);

        if (!packet_e) {
            return std::make_unexpected<dhcp_configuration>(packet_e.error());
        }

        auto& packet = *packet_e;

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
        auto status = parent->finalize_packet(interface, packet);
        if (!status) {
            return std::make_unexpected<dhcp_configuration>(status.error());
        }
    }

    logging::logf(logging::log_level::TRACE, "dhcp: Sent request\n");

    // 2. Receive DHCP Offer

    network::ip::address offer_address;
    network::ip::address server_address;
    network::ip::address gateway_address;
    network::ip::address dns_address;

    bool gateway = false;
    bool dns     = false;

    {
        while (true) {
            logging::logf(logging::log_level::TRACE, "dhcp: Wait for answer\n");

            if (packets.empty()) {
                listen_queue.wait();
            }

            auto packet = packets.back();
            packets.pop_back();

            auto* dhcp_header = reinterpret_cast<network::dhcp::header*>(packet->payload + packet->tag(3));

            if (dhcp_header->xid == 0x66666666 && dhcp_header->op == 0x2) {
                logging::logf(logging::log_level::TRACE, "dhcp: Received DHCP answer\n");

                auto* options = packet->payload + packet->tag(3) + sizeof(network::dhcp::header);

                bool dhcp_offer = false;

                auto cookie = (options[0] << 24) + (options[1] << 16) + (options[2] << 8) + options[3];
                if (cookie != 0x63825363) {
                    logging::logf(logging::log_level::TRACE, "dhcp: Received wrong magic cookie\n");
                    continue;
                }

                size_t i = 4;
                while (true) {
                    auto id = options[i++];

                    // End of options
                    if (id == 255) {
                        break;
                    }

                    // Pad
                    if (id == 0) {
                        continue;
                    }

                    auto n = options[i++];

                    if (!n) {
                        continue;
                    }

                    if (id == 53) {
                        if (options[i++] == 2) {
                            dhcp_offer = true;
                        }
                    } else if (id == 3) {
                        gateway_address = network::ip::make_address(options[i], options[i + 1], options[i + 2], options[i + 3]);
                        i += n;
                        gateway = true;
                    } else if (id == 6) {
                        dns_address = network::ip::make_address(options[i], options[i + 1], options[i + 2], options[i + 3]);
                        i += n;
                        dns = true;
                    } else {
                        i += n;
                    }
                }

                if (!dhcp_offer) {
                    logging::logf(logging::log_level::TRACE, "dhcp: Received wrong DHCP message type\n");
                    continue;
                }

                offer_address  = switch_endian_32(dhcp_header->your_ip);
                server_address = switch_endian_32(dhcp_header->server_ip);

                break;
            }
        }
    }

    logging::logf(logging::log_level::TRACE, "dhcp: Received DHCP Offer\n");
    logging::logf(logging::log_level::TRACE, "dhcp:          From %h\n", size_t(server_address.raw_address));
    logging::logf(logging::log_level::TRACE, "dhcp:            IP %h\n", size_t(offer_address.raw_address));
    logging::logf(logging::log_level::TRACE, "dhcp:           DNS %b\n", dns);
    logging::logf(logging::log_level::TRACE, "dhcp:       Gateway %b\n", gateway);

    // 3. Send DHCP Request

    {
        // Ask the UDP layer to craft a packet
        auto payload_size = sizeof(network::dhcp::header) + 20;
        network::udp::kernel_packet_descriptor udp_desc{payload_size, 68, 67, server_address};

        auto packet_e = parent->kernel_prepare_packet(interface, udp_desc);

        if (!packet_e) {
            return std::make_unexpected<dhcp_configuration>(packet_e.error());
        }

        auto& packet = *packet_e;

        ::prepare_packet(*packet, interface);

        auto* dhcp_header = reinterpret_cast<network::dhcp::header*>(packet->payload + packet->tag(3));

        dhcp_header->op        = 1;                                            // This is a request
        dhcp_header->xid       = 0x66666666;                                   // Our identification
        dhcp_header->flags     = switch_endian_16(0x8000);                     // We don't know our IP address for now
        dhcp_header->server_ip = switch_endian_32(server_address.raw_address); // Address ip of the server

        auto* options = packet->payload + packet->tag(3) + sizeof(network::dhcp::header);

        // Option 0: Magic cookie
        options[0] = 99;
        options[1] = 130;
        options[2] = 83;
        options[3] = 99;

        // Option 1: DHCP Request
        options[4] = 53; // DHCP Message Type
        options[5] = 1;  // Length 1
        options[6] = 3;  // DHCP Request

        // Option 2: Request IP address
        options[7]  = 50;
        options[8]  = 4;
        options[9]  = offer_address(0);
        options[10] = offer_address(1);
        options[11] = offer_address(2);
        options[12] = offer_address(3);

        // Option 3: DHCP server
        options[13] = 54;
        options[14] = 4;
        options[15] = server_address(0);
        options[16] = server_address(1);
        options[17] = server_address(2);
        options[18] = server_address(3);

        // End of options
        options[19] = 255;

        // Finalize the UDP packet
        auto status = parent->finalize_packet(interface, packet);
        if (!status) {
            return std::make_unexpected<dhcp_configuration>(status.error());
        }
    }

    // 4. Receive DHCP Acknowledge

    {
        while (true) {
            if (packets.empty()) {
                listen_queue.wait();
            }

            auto packet = packets.back();
            packets.pop_back();

            auto* dhcp_header = reinterpret_cast<network::dhcp::header*>(packet->payload + packet->tag(3));

            if (dhcp_header->xid == 0x66666666 && dhcp_header->op == 0x2) {
                logging::logf(logging::log_level::TRACE, "dhcp: Received DHCP answer\n");

                auto* options = packet->payload + packet->tag(3) + sizeof(network::dhcp::header);

                auto cookie = (options[0] << 24) + (options[1] << 16) + (options[2] << 8) + options[3];
                if (cookie != 0x63825363) {
                    logging::logf(logging::log_level::TRACE, "dhcp: Received wrong magic cookie\n");
                    continue;
                }

                bool dhcp_ack = false;

                size_t i = 4;
                while (true) {
                    auto id = options[i++];

                    // End of options
                    if (id == 255) {
                        break;
                    }

                    // Pad
                    if (id == 0) {
                        continue;
                    }

                    auto n = options[i++];

                    if (!n) {
                        continue;
                    }

                    if (id == 53) {
                        if (options[i] == 5 || options[i] == 6) {
                            dhcp_ack = true;
                        }

                        ++i;

                        continue;
                    }
                }

                if (!dhcp_ack) {
                    logging::logf(logging::log_level::TRACE, "dhcp: Received wrong DHCP message type\n");
                    continue;
                }

                logging::logf(logging::log_level::TRACE, "dhcp: Received DHCP Ack\n");

                break;
            }
        }
    }

    listening = false;

    dhcp_configuration conf;

    conf.dns        = dns;
    conf.gateway    = gateway;
    conf.ip_address = offer_address;

    if (dns) {
        conf.dns_address = dns_address;
    }

    if (gateway) {
        conf.gateway_address = gateway_address;
    }

    logging::logf(logging::log_level::TRACE, "dhcp: Finished discovery\n");

    return {conf};
}
