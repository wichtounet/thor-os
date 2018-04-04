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

namespace {

int wget_http(const std::string& url){
    auto parts = std::split(url, '/');

    if(parts.size() < 2){
        tlib::print_line("wget: Invalid url");
        return 1;
    }

    if(parts[0] == "http:"){
        auto& domain = parts[1];

        tlib::ip::address ip;
        if (tlib::dns::is_ip(domain)) {
            auto ip_parts = std::split(domain, '.');

            ip = tlib::ip::make_address(std::atoui(ip_parts[0]), std::atoui(ip_parts[1]), std::atoui(ip_parts[2]), std::atoui(ip_parts[3]));
        } else {
            auto ip_result = tlib::dns::resolve(domain);

            if(!ip_result){
                tlib::print_line("wget: cannot resolve the domain");
                return 1;
            }

            ip = *ip_result;
        }

        tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::STREAM, tlib::socket_protocol::TCP);

        sock.connect(ip, 80);
        sock.listen(true);

        if (!sock) {
            tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
            return 1;
        }

        std::string message;

        message += "GET ";

        if (parts.size() < 3) {
            message += '/';
        } else {
            for (size_t i = 2; i < parts.size(); ++i) {
                message += '/';
                message += parts[i];
            }
        }

        message += " HTTP/1.1\r\n";
        message += "Host: ";
        message += domain;
        message += "\r\n";
        message += "Accept: text/html text/plain\r\n";
        message += "User-Agent: wget (Thor OS)\r\n";
        message += "\r\n";

        sock.send(message.c_str(), message.size());

        char message_buffer[2049];
        auto size = sock.receive(message_buffer, 2048, 2000);
        if (!sock) {
            if (sock.error() == std::ERROR_SOCKET_TIMEOUT) {
                tlib::printf("Timeout\n");
                return 1;
            }

            tlib::printf("nc: receive error: %s\n", std::error_message(sock.error()));
            return 1;
        } else {
            message_buffer[size] = '\0';
            tlib::print(message_buffer);
        }

        sock.listen(false);

        if (!sock) {
            tlib::printf("nc: socket error: %s\n", std::error_message(sock.error()));
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
