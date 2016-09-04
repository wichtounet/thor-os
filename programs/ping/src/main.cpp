//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/net.hpp>

namespace {

} // end of anonymous namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        tlib::print_line("usage: ping address_ip");
        return 1;
    }

    std::string ip(argv[1]);
    auto ip_parts = std::split(ip, '.');

    if (ip_parts.size() != 4) {
        tlib::print_line("Invalid address IP");
        return 1;
    }

    auto socket = tlib::socket_open(tlib::socket_domain::AF_INET, tlib::socket_type::RAW, tlib::socket_protocol::ICMP);

    if (!socket) {
        tlib::printf("ls: socket_open error: %s\n", std::error_message(socket.error()));
        return 1;
    }

    auto status = tlib::listen(*socket, true);
    if (!status) {
        tlib::printf("ping: listen error: %s\n", std::error_message(status.error()));
        return 1;
    }

    tlib::icmp::packet_descriptor desc;
    desc.payload_size = 0;
    desc.target_ip    = tlib::ip::make_address(std::atoui(ip_parts[0]), std::atoui(ip_parts[1]), std::atoui(ip_parts[2]), std::atoui(ip_parts[3]));
    desc.type         = tlib::icmp::type::ECHO_REQUEST;
    desc.code         = 0;

    auto packet = tlib::prepare_packet(*socket, &desc);

    if (!packet) {
        tlib::printf("ping: prepare_packet error: %s\n", std::error_message(packet.error()));
        return 1;
    }

    auto* command_header = reinterpret_cast<tlib::icmp::echo_request_header*>(packet->payload + packet->index);

    command_header->identifier = 0x666;
    command_header->sequence   = 0x1;

    status = tlib::finalize_packet(*socket, *packet);
    if (!status) {
        tlib::printf("ping: finalize_packet error: %s\n", std::error_message(status.error()));
        return 1;
    }

    auto p = tlib::wait_for_packet(*socket);
    if (!p) {
        tlib::printf("ping: wait_for_packet error: %s\n", std::error_message(p.error()));
        return 1;
    }

    //TODO Wait for replies

    status = tlib::listen(*socket, false);
    if (!status) {
        tlib::printf("ping: listen error: %s\n", std::error_message(status.error()));
        return 1;
    }

    tlib::socket_close(*socket);

    return 0;
}
