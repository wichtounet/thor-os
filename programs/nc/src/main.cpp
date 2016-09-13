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
#include <tlib/dns.hpp>

namespace {



} // end of anonymous namespace

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

    sock.listen(false);

    return 0;
}
