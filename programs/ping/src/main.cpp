//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/net.hpp>

namespace {

static constexpr const size_t N          = 4;
static constexpr const size_t timeout_ms = 2000;

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

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::RAW, tlib::socket_protocol::ICMP);

    if (!sock) {
        tlib::printf("ls: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    sock.listen(true);

    if (!sock) {
        tlib::printf("ls: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    tlib::icmp::packet_descriptor desc;
    desc.payload_size = 0;
    desc.target_ip    = tlib::ip::make_address(std::atoui(ip_parts[0]), std::atoui(ip_parts[1]), std::atoui(ip_parts[2]), std::atoui(ip_parts[3]));
    desc.type         = tlib::icmp::type::ECHO_REQUEST;
    desc.code         = 0;

    for (size_t i = 0; i < N; ++i) {
        auto packet = sock.prepare_packet(&desc);

        if (!sock) {
            if (sock.error() == std::ERROR_SOCKET_TIMEOUT) {
                tlib::printf("Unable to resolve MAC address for target IP\n");
                return 1;
            }

            tlib::printf("ping: prepare_packet error: %s\n", std::error_message(sock.error()));
            return 1;
        }

        auto* command_header = reinterpret_cast<tlib::icmp::echo_request_header*>(packet.payload + packet.index);

        command_header->identifier = 0x666;
        command_header->sequence   = 0x1 + i;

        sock.finalize_packet(packet);

        if (!sock) {
            tlib::printf("ping: finalize_packet error: %s\n", std::error_message(sock.error()));
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

            bool handled = false;

            auto p = sock.wait_for_packet(remaining);
            if (!sock) {
                if (sock.error() == std::ERROR_SOCKET_TIMEOUT) {
                    tlib::printf("%s unreachable\n", ip.c_str());
                    handled = true;
                    sock.clear();
                } else {
                    tlib::printf("ping: wait_for_packet error: %s\n", std::error_message(sock.error()));
                    return 1;
                }
            } else {
                auto* icmp_header = reinterpret_cast<tlib::icmp::header*>(p.payload + p.index);

                auto command_type = static_cast<tlib::icmp::type>(icmp_header->type);

                if (command_type == tlib::icmp::type::ECHO_REPLY) {
                    tlib::printf("Reply received from %s\n", ip.c_str());
                    handled = true;
                }
            }

            if (handled) {
                break;
            }

            after = tlib::ms_time();
        }

        if (i < N - 1) {
            tlib::sleep_ms(1000);
        }
    }

    sock.listen(false);

    return 0;
}
