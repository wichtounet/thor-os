//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/net.hpp"
#include "tlib/malloc.hpp"

tlib::packet::packet()
        : fd(0), payload(nullptr), index(0) {
    //Nothing else to init
}

tlib::packet::packet(packet&& rhs)
        : fd(rhs.fd), payload(rhs.payload), index(rhs.index) {
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
    asm volatile("mov rax, 0xB00; mov rbx, %[domain]; mov rcx, %[type]; mov rdx, %[protocol]; int 50; mov %[fd], rax"
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
    asm volatile("mov rax, 0xB01; mov rbx, %[fd]; int 50;"
                 : /* No outputs */
                 : [fd] "g"(fd)
                 : "rax", "rbx");
}

std::expected<tlib::packet> tlib::prepare_packet(size_t socket_fd, void* desc) {
    auto buffer = malloc(2048);

    int64_t fd;
    uint64_t index;
    asm volatile("mov rax, 0xB02; mov rbx, %[socket]; mov rcx, %[desc]; mov rdx, %[buffer]; int 50; mov %[fd], rax; mov %[index], rbx;"
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
    asm volatile("mov rax, 0xB03; mov rbx, %[socket]; mov rcx, %[packet]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [packet] "g"(packet_fd)
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_expected_from_error<void, size_t>(-code);
    } else {
        return std::make_expected();
    }
}

std::expected<void> tlib::send(size_t socket_fd, const char* buffer, size_t n) {
    auto* target_buffer = new char[2048];

    int64_t code;
    asm volatile("mov rax, 0xB0B; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[n]; mov rsi, %[target_buffer]; int 50; mov %[code], rax;"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [n] "g" (n), [target_buffer] "g"(reinterpret_cast<size_t>(target_buffer))
                 : "rax", "rbx", "rcx", "rdx", "rsi");

    delete[] target_buffer;

    if (code < 0) {
        return std::make_unexpected<void, size_t>(-code);
    } else {
        return {};
    }
}

std::expected<void> tlib::send_to(size_t socket_fd, const char* buffer, size_t n, void* address) {
    auto* target_buffer = new char[2048];

    int64_t code;
    asm volatile("mov rax, 0xB13; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[n]; mov rsi, %[target_buffer]; mov rdi, %[address]; int 50; mov %[code], rax;"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [n] "g" (n), [target_buffer] "g"(reinterpret_cast<size_t>(target_buffer)), [address] "g"(reinterpret_cast<size_t>(address))
                 : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");

    delete[] target_buffer;

    if (code < 0) {
        return std::make_unexpected<void, size_t>(-code);
    } else {
        return {};
    }
}

std::expected<size_t> tlib::receive(size_t socket_fd, char* buffer, size_t n) {
    int64_t code;
    asm volatile("mov rax, 0xB10; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[n]; int 50; mov %[code], rax;"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [n] "g" (n)
                 : "rax", "rbx", "rcx", "rdx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return code;
    }
}

std::expected<size_t> tlib::receive(size_t socket_fd, char* buffer, size_t n, size_t ms) {
    int64_t code;
    asm volatile("mov rax, 0xB0C; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[n]; mov rsi, %[ms]; int 50; mov %[code], rax;"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [n] "g" (n), [ms] "g" (ms)
                 : "rax", "rbx", "rcx", "rdx", "rsi");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return code;
    }
}

std::expected<size_t> tlib::receive_from(size_t socket_fd, char* buffer, size_t n, void* address) {
    int64_t code;
    asm volatile("mov rax, 0xB11; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[n]; mov rsi, %[address]; int 50; mov %[code], rax;"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [n] "g" (n), [address] "g" (reinterpret_cast<size_t>(address))
                 : "rax", "rbx", "rcx", "rdx", "rsi");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return code;
    }
}

std::expected<size_t> tlib::receive_from(size_t socket_fd, char* buffer, size_t n, size_t ms, void* address) {
    int64_t code;
    asm volatile("mov rax, 0xB12; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[n]; mov rsi, %[ms]; mov rdi, %[address]; int 50; mov %[code], rax;"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [n] "g" (n), [ms] "g" (ms), [address] "g" (reinterpret_cast<size_t>(address))
                 : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return code;
    }
}

std::expected<void> tlib::listen(size_t socket_fd, bool l) {
    int64_t code;
    asm volatile("mov rax, 0xB04; mov rbx, %[socket]; mov rcx, %[listen]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [listen] "g"(size_t(l))
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_expected_from_error<void, size_t>(-code);
    } else {
        return std::make_expected();
    }
}

std::expected<size_t> tlib::client_bind(size_t socket_fd, tlib::ip::address server) {
    int64_t code;
    asm volatile("mov rax, 0xB07; mov rbx, %[socket]; mov rcx, %[ip]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [ip] "g" (size_t(server.raw_address))
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> tlib::client_bind(size_t socket_fd, tlib::ip::address server, size_t port) {
    int64_t code;
    asm volatile("mov rax, 0xB0D; mov rbx, %[socket]; mov rcx, %[ip]; mov rdx, %[port]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [ip] "g" (size_t(server.raw_address)), [port] "g" (port)
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<void> tlib::server_bind(size_t socket_fd, tlib::ip::address server) {
    int64_t code;
    asm volatile("mov rax, 0xB0E; mov rbx, %[socket]; mov rcx, %[ip]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [ip] "g" (size_t(server.raw_address))
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_unexpected<void, size_t>(-code);
    } else {
        return {};
    }
}

std::expected<void> tlib::server_bind(size_t socket_fd, tlib::ip::address server, size_t port) {
    int64_t code;
    asm volatile("mov rax, 0xB0F; mov rbx, %[socket]; mov rcx, %[ip]; mov rdx, %[port]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [ip] "g" (size_t(server.raw_address)), [port] "g" (port)
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_unexpected<void, size_t>(-code);
    } else {
        return {};
    }
}

std::expected<void> tlib::client_unbind(size_t socket_fd) {
    int64_t code;
    asm volatile("mov rax, 0xB0A; mov rbx, %[socket]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd)
                 : "rax", "rbx");

    if (code < 0) {
        return std::make_unexpected<void, size_t>(-code);
    } else {
        return {};
    }
}

std::expected<size_t> tlib::connect(size_t socket_fd, tlib::ip::address server, size_t port) {
    int64_t code;
    asm volatile("mov rax, 0xB08; mov rbx, %[socket]; mov rcx, %[ip]; mov rdx, %[port]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [ip] "g"(size_t(server.raw_address)), [port] "g"(port)
                 : "rax", "rbx", "rcx", "rdx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<void> tlib::server_start(size_t socket_fd, tlib::ip::address server, size_t port) {
    int64_t code;
    asm volatile("mov rax, 0xB14; mov rbx, %[socket]; mov rcx, %[ip]; mov rdx, %[port]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [ip] "g"(size_t(server.raw_address)), [port] "g"(port)
                 : "rax", "rbx", "rcx", "rdx");

    if (code < 0) {
        return std::make_unexpected<void, size_t>(-code);
    } else {
        return {};
    }
}

std::expected<size_t> tlib::accept(size_t socket_fd) {
    int64_t code;
    asm volatile("mov rax, 0xB16; mov rbx, %[socket]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd)
                 : "rax", "rbx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return code;
    }
}

std::expected<size_t> tlib::accept(size_t socket_fd, size_t ms) {
    int64_t code;
    asm volatile("mov rax, 0xB17; mov rbx, %[socket]; mov rcx, %[ms]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd), [ms] "g"(ms)
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        return std::make_unexpected<size_t, size_t>(-code);
    } else {
        return code;
    }
}

std::expected<void> tlib::disconnect(size_t socket_fd) {
    int64_t code;
    asm volatile("mov rax, 0xB09; mov rbx, %[socket]; int 50; mov %[code], rax"
                 : [code] "=m"(code)
                 : [socket] "g"(socket_fd)
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
    asm volatile("mov rax, 0xB05; mov rbx, %[socket]; mov rcx, %[buffer]; int 50; mov %[code], rax; mov %[payload], rbx;"
                 : [payload] "=m"(payload), [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer))
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        free(buffer);
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
    asm volatile("mov rax, 0xB06; mov rbx, %[socket]; mov rcx, %[buffer]; mov rdx, %[ms]; int 50; mov %[code], rax; mov %[payload], rbx;"
                 : [payload] "=m"(payload), [code] "=m"(code)
                 : [socket] "g"(socket_fd), [buffer] "g"(reinterpret_cast<size_t>(buffer)), [ms] "g"(ms)
                 : "rax", "rbx", "rcx");

    if (code < 0) {
        free(buffer);
        return std::make_expected_from_error<packet, size_t>(-code);
    } else {
        tlib::packet p;
        p.index   = code;
        p.payload = reinterpret_cast<char*>(payload);
        return std::make_expected<packet>(std::move(p));
    }
}

tlib::socket::socket() : fd(0), error_code(0) {
    // Nothing else to init
}

tlib::socket::socket(socket_domain domain, socket_type type, socket_protocol protocol)
        : domain(domain), type(type), protocol(protocol), fd(0), error_code(0) {
    auto open_status = tlib::socket_open(domain, type, protocol);

    if (open_status.valid()) {
        fd = *open_status;
    } else {
        error_code = open_status.error();
    }
}

tlib::socket::socket(tlib::socket&& rhs)
        : domain(rhs.domain), type(rhs.type), protocol(rhs.protocol), fd(rhs.fd), error_code(rhs.error_code), _connected(rhs._connected), _bound(rhs._bound) {
    // This needs to be done so that the rhs will not do anything on
    // destroy
    rhs.fd = 0;
    rhs._connected = false;
    rhs._bound = false;
}

tlib::socket& tlib::socket::operator=(tlib::socket&& rhs){
    if(this != &rhs){
        this->domain     = rhs.domain;
        this->type       = rhs.type;
        this->protocol   = rhs.protocol;
        this->fd         = rhs.fd;
        this->error_code = rhs.error_code;
        this->_connected = rhs._connected;
        this->_bound     = rhs._bound;

        // This needs to be done so that the rhs will not do anything on
        // destroy
        rhs.fd = 0;
        rhs._connected = false;
        rhs._bound = false;
    }

    return *this;
}

tlib::socket::~socket() {
    if (connected()) {
        disconnect();
    }

    if (bound()) {
        client_unbind();
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

bool tlib::socket::bound() const {
    return _bound;
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

void tlib::socket::client_bind(tlib::ip::address server) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::client_bind(fd, server);
    if (status) {
        _bound = true;
    } else {
        _bound = false;
        error_code = status.error();
    }
}

void tlib::socket::client_bind(tlib::ip::address server, size_t port) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::client_bind(fd, server, port);
    if (status) {
        _bound = true;
    } else {
        error_code = status.error();
        _bound = false;
    }
}

void tlib::socket::server_bind(tlib::ip::address server) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::server_bind(fd, server);
    if (status) {
        _bound = true;
    } else {
        _bound = false;
        error_code = status.error();
    }
}

void tlib::socket::server_bind(tlib::ip::address server, size_t port) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::server_bind(fd, server, port);
    if (status) {
        _bound = true;
    } else {
        error_code = status.error();
        _bound = false;
    }
}

void tlib::socket::client_unbind() {
    if (!good() || !open()) {
        return;
    }

    if (!bound()) {
        return;
    }

    auto status = tlib::client_unbind(fd);

    if (status) {
        _bound = false;
    } else {
        error_code = status.error();
    }
}

void tlib::socket::connect(tlib::ip::address server, size_t port) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::connect(fd, server, port);
    if (status) {
        _connected = true;
    } else {
        error_code = status.error();
        _connected = false;
    }
}

void tlib::socket::server_start(tlib::ip::address server, size_t port) {
    if (!good() || !open()) {
        return;
    }

    auto status = tlib::server_start(fd, server, port);
    if (status) {
        _connected = true;
    } else {
        error_code = status.error();
        _connected = false;
    }
}

tlib::socket tlib::socket::accept() {
    if (!good() || !open()) {
        return {};
    }

    auto status = tlib::accept(fd);
    if (status) {
        tlib::socket sock;

        sock.fd = *status;

        sock.domain   = domain;
        sock.type     = type;
        sock.protocol = protocol;

        sock._connected = true;
        sock._bound     = false;
        sock.error_code = 0;

        return sock;
    } else {
        error_code = status.error();
    }

    return {};
}

tlib::socket tlib::socket::accept(size_t ms) {
    if (!good() || !open()) {
        return {};
    }

    auto status = tlib::accept(fd, ms);
    if (status) {
        //TODO
    } else {
        error_code = status.error();
    }

    return {};
}

void tlib::socket::disconnect() {
    if (!good() || !open()) {
        return;
    }

    if (!connected()) {
        return;
    }

    auto status = tlib::disconnect(fd);

    if (status) {
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

void tlib::socket::send(const char* buffer, size_t n) {
    if (!good() || !open()) {
        return;
    }

    if (type == socket_type::DGRAM) {
        if (!bound()) {
            return;
        }
    } else if (type == socket_type::STREAM) {
        if (!connected()) {
            return;
        }
    } else {
        return;
    }

    auto p = tlib::send(fd, buffer, n);
    if (!p) {
        error_code = p.error();
    }
}

void tlib::socket::send_to(const char* buffer, size_t n, void* address) {
    if (!good() || !open()) {
        return;
    }

    if (type == socket_type::DGRAM) {
        if (!bound()) {
            return;
        }
    } else if (type == socket_type::STREAM) {
        if (!connected()) {
            return;
        }
    } else {
        return;
    }

    auto p = tlib::send_to(fd, buffer, n, address);
    if (!p) {
        error_code = p.error();
    }
}

size_t tlib::socket::receive(char* buffer, size_t n) {
    if (!good() || !open()) {
        return 0;
    }

    if (type == socket_type::DGRAM) {
        if (!bound()) {
            return 0;
        }
    } else if (type == socket_type::STREAM) {
        if (!connected()) {
            return 0;
        }
    } else {
        return 0;
    }

    auto p = tlib::receive(fd, buffer, n);
    if (!p) {
        error_code = p.error();
        return 0;
    } else {
        return *p;
    }
}

size_t tlib::socket::receive(char* buffer, size_t n, size_t ms) {
    if (!good() || !open()) {
        return 0;
    }

    if (type == socket_type::DGRAM) {
        if (!bound()) {
            return 0;
        }
    } else if (type == socket_type::STREAM) {
        if (!connected()) {
            return 0;
        }
    } else {
        return 0;
    }

    auto p = tlib::receive(fd, buffer, n, ms);
    if (!p) {
        error_code = p.error();
        return 0;
    } else {
        return *p;
    }
}

size_t tlib::socket::receive_from(char* buffer, size_t n, void* address) {
    if (!good() || !open()) {
        return 0;
    }

    if (type == socket_type::DGRAM) {
        if (!bound()) {
            return 0;
        }
    } else if (type == socket_type::STREAM) {
        if (!connected()) {
            return 0;
        }
    } else {
        return 0;
    }

    auto p = tlib::receive_from(fd, buffer, n, address);
    if (!p) {
        error_code = p.error();
        return 0;
    } else {
        return *p;
    }
}

size_t tlib::socket::receive_from(char* buffer, size_t n, size_t ms, void* address) {
    if (!good() || !open()) {
        return 0;
    }

    if (type == socket_type::DGRAM) {
        if (!bound()) {
            return 0;
        }
    } else if (type == socket_type::STREAM) {
        if (!connected()) {
            return 0;
        }
    } else {
        return 0;
    }

    auto p = tlib::receive_from(fd, buffer, n, ms, address);
    if (!p) {
        error_code = p.error();
        return 0;
    } else {
        return *p;
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
