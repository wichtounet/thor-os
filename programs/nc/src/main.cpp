//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/net.hpp>
#include <tlib/dns.hpp>

static constexpr const size_t timeout_ms = 5000;
static constexpr const size_t server_timeout_ms = 10000;

namespace {

std::string ip_to_str(tlib::ip::address ip){
    std::string value;
    value += std::to_string(ip(0));
    value += '.';
    value += std::to_string(ip(1));
    value += '.';
    value += std::to_string(ip(2));
    value += '.';
    value += std::to_string(ip(3));
    return value;
}

int netcat_tcp_client(const tlib::ip::address& server, size_t port){
    auto ip_str = ip_to_str(server);
    tlib::printf("netcat TCP client %s:%u\n", ip_str.c_str(), port);

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::STREAM, tlib::socket_protocol::TCP);

    sock.connect(server, port);
    sock.listen(true);

    if (!sock) {
        tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    tlib::printf("nc: wait 2 seconds\n");
    tlib::sleep_ms(2000);

    // Send a packet to the server

    tlib::printf("nc: send a message\n");

    {
        auto message = "THOR";

        sock.send(message, 4);

        if (!sock) {
            tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
            return 1;
        }
    }

    tlib::printf("nc: wait for message\n");

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

int netcat_udp_client(const tlib::ip::address& server, size_t port){
    auto ip_str = ip_to_str(server);
    tlib::printf("netcat UDP client %s:%u\n", ip_str.c_str(), port);

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::DGRAM, tlib::socket_protocol::UDP);

    sock.client_bind(server, port);
    sock.listen(true);

    if (!sock) {
        tlib::printf("nc: listen error: %s\n", std::error_message(sock.error()));
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
        tlib::printf("nc: listen error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    return 0;
}

int netcat_tcp_server(const tlib::ip::address& local, size_t port){
    auto ip_str = ip_to_str(local);
    tlib::printf("netcat TCP server %s:%u\n", ip_str.c_str(), port);

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::STREAM, tlib::socket_protocol::TCP);

    sock.server_start(local, port);

    if (!sock) {
        tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    auto child = sock.accept();

    if (!sock) {
        tlib::printf("nc: accept error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    if (!child) {
        tlib::printf("nc: accept error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    child.listen(true);

    tlib::printf("nc: Received connection\n");

    if (!child) {
        tlib::printf("nc: listen error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    // Listen for packets from the client

    char message_buffer[2049];

    auto before = tlib::ms_time();
    auto after  = before;

    while (true) {
        // Make sure we don't wait for more than the timeout
        if (after > before + server_timeout_ms) {
            break;
        }

        auto remaining = server_timeout_ms - (after - before);

        tlib::printf("nc: Wait for message\n");

        auto size = child.receive(message_buffer, 2048, remaining);
        if (!child) {
            // Simple timeout
            if (child.error() == std::ERROR_SOCKET_TIMEOUT) {
                child.clear();
                break;
            }

            // It may have disconnected in the meantime
            if (child.error() == std::ERROR_SOCKET_NOT_CONNECTED) {
                child.clear();
                break;
            }

            tlib::printf("nc: receive error: %s\n", std::error_message(child.error()));
            return 1;
        } else {
            message_buffer[size] = '\0';

            tlib::printf("nc: received message of size %u: %s\n", size, message_buffer);
            tlib::printf("nc: Send response back\n");

            child.send(message_buffer, size);

            if (!child) {
                tlib::printf("nc: send error: %s\n", std::error_message(child.error()));
                return 1;
            }
        }

        after = tlib::ms_time();
    }

    tlib::printf("nc: done... disconnecting\n");

    child.listen(false);
    child.clear();

    return 0;
}

int netcat_udp_server(const tlib::ip::address& local, size_t port){
    auto ip_str = ip_to_str(local);
    tlib::printf("netcat UDP server %s:%u\n", ip_str.c_str(), port);

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::DGRAM, tlib::socket_protocol::UDP);

    sock.server_bind(local, port);
    sock.listen(true);

    if (!sock) {
        tlib::printf("nc: listen error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    // Listen for packets from the client

    char message_buffer[2049];

    auto before = tlib::ms_time();
    auto after  = before;

    while (true) {
        // Make sure we don't wait for more than the timeout
        if (after > before + server_timeout_ms) {
            break;
        }

        auto remaining = server_timeout_ms - (after - before);

        tlib::inet_address address;

        auto size = sock.receive_from(message_buffer, 2048, remaining, &address);
        if (!sock) {
            if (sock.error() == std::ERROR_SOCKET_TIMEOUT) {
                sock.clear();
                break;
            }

            tlib::printf("nc: receive_from error: %s\n", std::error_message(sock.error()));
            return 1;
        } else {
            message_buffer[size] = '\0';
            tlib::print(message_buffer);

            sock.send_to(message_buffer, size, &address);

            if (!sock) {
                tlib::printf("nc: send_to error: %s\n", std::error_message(sock.error()));
                return 1;
            }
        }

        after = tlib::ms_time();
    }

    sock.listen(false);

    if (!sock) {
        tlib::printf("nc: listen error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    return 0;
}

} // end of anonymous namespace

int main(int argc, char* argv[]) {

    bool udp = false;
    bool server = false;

    size_t i = 1;
    for(; i < size_t(argc); ++i){
        std::string param(argv[i]);

        if(param == "-u"){
            udp = true;
        } else if(param == "-l"){
            server = true;
        } else {
            break;
        }
    }

    if(server){
        if (argc - i != 2) {
            tlib::print_line("usage: nc -l local port");
            return 1;
        }

        std::string local(argv[i]);
        std::string port_str(argv[i + 1]);

        auto port = std::atoui(port_str);

        auto ip_parts = std::split(local, '.');

        if (ip_parts.size() != 4) {
            tlib::print_line("Invalid address IP for the local interface");
            return 1;
        }

        auto local_ip = tlib::ip::make_address(std::atoui(ip_parts[0]), std::atoui(ip_parts[1]), std::atoui(ip_parts[2]), std::atoui(ip_parts[3]));

        if (udp) {
            return netcat_udp_server(local_ip, port);
        } else {
            return netcat_tcp_server(local_ip, port);
        }
    } else {
        if (argc - i != 2) {
            tlib::print_line("usage: nc server port");
            return 1;
        }

        std::string server(argv[i]);
        std::string port_str(argv[i + 1]);

        auto port = std::atoui(port_str);

        auto ip_parts = std::split(server, '.');

        if (ip_parts.size() != 4) {
            tlib::print_line("Invalid address IP for the server");
            return 1;
        }

        auto server_ip = tlib::ip::make_address(std::atoui(ip_parts[0]), std::atoui(ip_parts[1]), std::atoui(ip_parts[2]), std::atoui(ip_parts[3]));

        if (udp) {
            return netcat_udp_client(server_ip, port);
        } else {
            return netcat_tcp_client(server_ip, port);
        }
    }
}
