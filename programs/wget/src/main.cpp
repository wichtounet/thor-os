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

int wget_http(const std::string& url){
    auto parts = std::split(url, '/');

    if(parts.size() < 2){
        tlib::print_line("wget: Invalid url");
        return 1;
    }

    if(parts[0] == "http:"){
        auto& domain = parts[1];

        auto ip = tlib::dns::resolve(domain);

        if(ip){
            tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::STREAM, tlib::socket_protocol::TCP);

            sock.connect(*ip, 80);
            sock.listen(true);

            if (!sock) {
                tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
                return 1;
            }

            //TODO

            sock.listen(false);

            if (!sock) {
                tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
                return 1;
            }
        } else {
            tlib::print_line("wget: cannot resolve the domain");
            return 1;
        }
    } else {
        tlib::print_line("wget: The given protocol is not support");
        return 1;
    }

    return 0;
}

} // end of anonymous namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        tlib::print_line("usage: wget url");
        return 1;
    }

    std::string url(argv[1]);
    return wget_http(url);
}
