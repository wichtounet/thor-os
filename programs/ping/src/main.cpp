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
        tlib::print_line("usage: ping address_ip");
        return 1;
    }

    std::string ip(argv[0]);

    auto socket = tlib::socket_open(tlib::socket_domain::AF_INET, tlib::socket_type::RAW, tlib::socket_protocol::ICMP);

    if(!socket){
        tlib::printf("ls: socket_open error: %s\n", std::error_message(socket.error()));
        return 1;
    }

    tlib::socket_close(*socket);

    return 0;
}
