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

int main(int argc, char* argv[]){
    if(argc != 2){
        print_line("usage: ping address_ip");
        return 1;
    }

    std::string ip(argv[0]);

    auto socket = socket_open(network::socket_domain::AF_INET, network::socket_type::RAW, network::socket_protocol::ICMP);

    if(!socket){
        printf("ls: socket_open error: %s\n", std::error_message(socket.error()));
        return 1;
    }

    socket_close(*socket);

    return 0;
}
