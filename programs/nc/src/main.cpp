//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>

#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/net.hpp>
#include <tlib/dns.hpp>

static constexpr const size_t timeout_ms = 5000;

using flag_data_offset = std::bit_field<uint16_t, uint8_t, 12, 4>;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        tlib::print_line("usage: nc server port");
        return 1;
    }

    std::string server(argv[1]);
    std::string port_str(argv[2]);
    auto port = std::atoui(port_str);

    auto ip_parts = std::split(server, '.');

    if (ip_parts.size() != 4) {
        tlib::print_line("Invalid address IP for the server");
        return 1;
    }

    auto server_ip = tlib::ip::make_address(std::atoui(ip_parts[0]), std::atoui(ip_parts[1]), std::atoui(ip_parts[2]), std::atoui(ip_parts[3]));

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::STREAM, tlib::socket_protocol::TCP);

    sock.connect(server_ip, port);
    sock.listen(true);

    if (!sock) {
        tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    // Send a packet to the server

    {
        auto message = "THOR";

        sock.send(message, 4);

        if (!sock) {
            tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
            return 1;
        }
    }

    // Listen for packets from the server

    char message_buffer[2049];

    auto before = tlib::ms_time();
    auto after  = before;

    while (true) {
        // Make sure we don't wait for more than the timeout
        if (after > before + timeout_ms) {
            break;
        }

        auto remaining = timeout_ms - (after - before);

        auto size = sock.receive(message_buffer, 2048, remaining);
        if (!sock) {
            if (sock.error() == std::ERROR_SOCKET_TIMEOUT) {
                sock.clear();
                break;
            }

            tlib::printf("nc: receive error: %s\n", std::error_message(sock.error()));
            return 1;
        } else {
            message_buffer[size] = '\0';
            tlib::print(message_buffer);
        }

        after = tlib::ms_time();
    }

    sock.listen(false);

    if (!sock) {
        tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    return 0;
}
