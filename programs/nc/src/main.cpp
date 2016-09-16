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

    tlib::tcp::packet_descriptor desc;
    desc.payload_size = 4;

    auto packet = sock.prepare_packet(&desc);

    if (!sock) {
        tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    auto* payload = reinterpret_cast<char*>(packet.payload + packet.index);
    payload[0]    = 'T';
    payload[1]    = 'H';
    payload[2]    = 'O';
    payload[3]    = 'R';

    sock.finalize_packet(packet);

    if (!sock) {
        tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    auto before = tlib::ms_time();
    auto after  = before;

    while (true) {
        // Make sure we don't wait for more than the timeout
        if (after > before + timeout_ms) {
            break;
        }

        auto remaining = timeout_ms - (after - before);

        auto p = sock.wait_for_packet(remaining);
        if (!sock) {
            if (sock.error() == std::ERROR_SOCKET_TIMEOUT) {
                sock.clear();
                break;
            }

            tlib::printf("nc: wait_for_packet error: %s\n", std::error_message(sock.error()));
            return 1;
        } else {
            auto* ip_header  = reinterpret_cast<tlib::ip::header*>(p.payload + sizeof(tlib::ethernet::header));
            auto* tcp_header = reinterpret_cast<tlib::tcp::header*>(p.payload + sizeof(tlib::ethernet::header) + sizeof(tlib::ip::header));
            auto* payload    = p.payload + p.index;

            auto tcp_flags = tlib::switch_endian_16(tcp_header->flags);

            auto ip_len          = (ip_header->version_ihl & 0xF) * 4;
            auto tcp_len         = tlib::switch_endian_16(ip_header->total_len) - ip_len;
            auto tcp_data_offset = *flag_data_offset(&tcp_flags) * 4;
            auto payload_len     = tcp_len - tcp_data_offset;

            for (size_t i = 0; i < payload_len; ++i) {
                tlib::print(payload[i]);
            }
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
