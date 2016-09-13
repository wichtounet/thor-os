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

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::STREAM, tlib::socket_protocol::TCP);

    sock.listen(true);

    if (!sock) {
        tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    sock.listen(false);

    return 0;
}
