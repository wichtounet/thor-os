//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/net.hpp"
#include "tlib/malloc.hpp"

tlib::packet::packet() : fd(0), payload(nullptr), index(0) {
    //Nothing else to init
}

tlib::packet::packet(packet&& rhs) : fd(rhs.fd), payload(rhs.payload), index(rhs.index) {
    rhs.payload = nullptr;
}

tlib::packet& tlib::packet::operator=(packet&& rhs) {
    if (this != &rhs) {
        this->fd      = rhs.fd;
        this->payload = rhs.payload;
        this->index   = rhs.index;

        rhs.payload = nullptr;
    }

    return *this;
}

tlib::packet::~packet() {
    if (payload) {
        tlib::free(payload);
    }
}

std::expected<size_t> tlib::socket_open(socket_domain domain, socket_type type, socket_protocol protocol) {
    int64_t fd;
    asm volatile("mov rax, 0x3000; mov rbx, %[domain]; mov rcx, %[type]; mov rdx, %[protocol]; int 50; mov %[fd], rax"
                 : [fd] "=m"(fd)
                 : [domain] "g"(static_cast<size_t>(domain)), [type] "g"(static_cast<size_t>(type)), [protocol] "g"(static_cast<size_t>(protocol))
                 : "rax", "rbx", "rcx", "rdx");

    if (fd < 0) {
        return std::make_expected_from_error<size_t, size_t>(-fd);
    } else {
        return std::make_expected<size_t>(fd);
    }
}

void tlib::socket_close(size_t fd) {
    asm volatile("mov rax, 0x3001; mov rbx, %[fd]; int 50;"
                 : /* No outputs */
                 : [fd] "g"(fd)
                 : "rax", "rbx");
}

std::expected<tlib::packet> tlib::prepare_packet(size_t socket_fd, void* desc) {
    auto buffer = malloc(2048);

    int64_t fd;
    uint64_t index;
    asm volatile("mov rax, 0x3002; mov rbx, %[socket]; mov rcx, %[desc]; mov rdx, %[buffer]; int 50; mov %[fd], rax; mov %[index], rbx;"
                 : [fd] "=m"(fd), [index] "=m"(index)
                 : [socket] "g"(socket_fd), [desc] "g"(reinterpret_cast<size_t>(desc)), [buffer] "g"(reinterpret_cast<size_t>(buffer))
                 : "rax", "rbx", "rcx", "rdx");

    if (fd < 0) {
        free(buffer);
        return std::make_expected_from_error<tlib::packet, size_t>(-fd);
    } else {
        tlib::packet p;
        p.fd      = fd;
        p.index   = index;
        p.payload = static_cast<char*>(buffer);
        return std::make_expected<packet>(std::move(p));
    }
}

std::expected<void> tlib::finalize_packet(size_t socket_fd, const tlib::packet& p) {
    auto packet_fd = p.fd;

    int64_t code;
    asm volatile("mov rax, 0x3003; mov rbx, %[socket]; mov rcx, %[packet]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [packet] "g"(packet_fd)
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_expected_from_error<void, size_t>(-code);
    } else {
        return std::make_expected();
    }
}

std::expected<void> tlib::listen(size_t socket_fd, bool l) {
    int64_t code;
    asm volatile("mov rax, 0x3004; mov rbx, %[socket]; mov rcx, %[listen]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [listen] "g"(size_t(l))
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_expected_from_error<void, size_t>(-code);
    } else {
        return std::make_expected();
    }
}

std::expected<size_t> tlib::client_bind(size_t socket_fd) {
    int64_t code;
    asm volatile("mov rax, 0x3007; mov rbx, %[socket]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd)
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> tlib::connect(size_t socket_fd, tlib::ip::address server, size_t port) {
    int64_t code;
    asm volatile("mov rax, 0x3008; mov rbx, %[socket]; mov rcx, %[ip]; mov rdx, %[port]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g" (socket_fd), [ip] "g" (size_t(server.raw_address)), [port] "g" (port)
                 : "rax", "rbx", "rcx", "rdx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<void> tlib::disconnect(size_t socket_fd) {
    int64_t code;
    asm volatile("mov rax, 0x3009; mov rbx, %[socket]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g" (socket_fd)
                 : "rax", "rbx");

    if (code < 0) {
        return std::make_unexpected<void, size_t>(-code);
    } else {
        return {};
    }
}

std::expected<tlib::packet> tlib::wait_for_packet(size_t socket_fd) {
    auto buffer = malloc(2048);

    int64_t code;
    uint64_t payload;
    asm volatile("mov rax, 0x3005; mov rbx, %[socket]; mov rcx, %[buffer]; int 50; mov %[code], rax; mov %[payload], rbx;"
                 : [payload] "=m"(payload), [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer))
                 : "rax", "rbx", "rcx");

    free(buffer);
    if (code < 0) {
        return std::make_expected_from_error<packet, size_t>(-code);
    } else {
        tlib::packet p;
        p.index   = code;
        p.payload = reinterpret_cast<char*>(payload);
        return std::make_expected<packet>(std::move(p));
    }
}

std::expected<tlib::packet> tlib::wait_for_packet(size_t socket_fd, size_t ms) {
    auto buffer = malloc(2048);

    int64_t code;
    uint64_t payload;
    asm volatile("mov rax, 0x3006; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[ms]; int 50; mov %[code], rax; mov %[payload], rbx;"
                 : [payload] "=m"(payload), [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [ms] "g"(ms)
                 : "rax", "rbx", "rcx");

    free(buffer);
    if (code < 0) {
        return std::make_expected_from_error<packet, size_t>(-code);
    } else {
        tlib::packet p;
        p.index   = code;
        p.payload = reinterpret_cast<char*>(payload);
        return std::make_expected<packet>(std::move(p));
    }
}

tlib::socket::socket(socket_domain domain, socket_type type, socket_protocol protocol)
        : domain(domain), type(type), protocol(protocol), fd(0), error_code(0) {
    auto open_status = tlib::socket_open(domain, type, protocol);

    if (open_status.valid()) {
        fd = *open_status;
    } else {
        error_code = open_status.error();
    }

    local_port = 0;
}

tlib::socket::~socket() {
    if(connected()){
        disconnect();
    }

    if (fd) {
        tlib::socket_close(fd);
    }
}

bool tlib::socket::open() const {
    return fd > 0;
}

bool tlib::socket::good() const {
    return error_code == 0;
}

bool tlib::socket::connected() const {
    return _connected;
}

tlib::socket::operator bool() {
    return good();
}

size_t tlib::socket::error() const {
    return error_code;
}

void tlib::socket::clear() {
    error_code = 0;
}

void tlib::socket::listen(bool l) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::listen(fd, l);
    if (!status) {
        error_code = status.error();
    }
}

void tlib::socket::client_bind() {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::client_bind(fd);
    if (!status) {
        error_code = status.error();
    }

    local_port = *status;
}

void tlib::socket::connect(tlib::ip::address server, size_t port) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::connect(fd, server, port);
    if (status) {
        _connected = true;
        local_port = *status;
    } else {
        error_code = status.error();
    }
}

void tlib::socket::disconnect() {
    if (!good() || !open()) {
        return;
    }

    if(!connected()){
        return;
    }

    auto status = tlib::disconnect(fd);

    if(status){
        _connected = false;
    } else {
        error_code = status.error();
    }
}

tlib::packet tlib::socket::prepare_packet(void* desc) {
    if (!good() || !open()) {
        return tlib::packet();
    }

    auto packet = tlib::prepare_packet(fd, desc);

    if (!packet) {
        error_code = packet.error();
        return tlib::packet();
    } else {
        return std::move(*packet);
    }
}

void tlib::socket::finalize_packet(const tlib::packet& p) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::finalize_packet(fd, p);
    if (!status) {
        error_code = status.error();
    }
}

tlib::packet tlib::socket::wait_for_packet() {
    if (!good() || !open()) {
        return tlib::packet();
    }

    auto p = tlib::wait_for_packet(fd);

    if (!p) {
        error_code = p.error();
        return tlib::packet();
    } else {
        return std::move(*p);
    }
}

tlib::packet tlib::socket::wait_for_packet(size_t ms) {
    if (!good() || !open()) {
        return tlib::packet();
    }

    auto p = tlib::wait_for_packet(fd, ms);

    if (!p) {
        error_code = p.error();
        return tlib::packet();
    } else {
        return std::move(*p);
    }
}
