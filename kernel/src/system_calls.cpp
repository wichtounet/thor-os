//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <array.hpp>

#include "system_calls.hpp"
#include "print.hpp"
#include "scheduler.hpp"
#include "timer.hpp"
#include "drivers/keyboard.hpp"
#include "stdio.hpp"
#include "acpi.hpp"
#include "drivers/rtc.hpp"
#include "kernel_utils.hpp"
#include "vesa.hpp"
#include "drivers/mouse.hpp"
#include "vfs/vfs.hpp"
#include "ioctl.hpp"
#include "net/network.hpp"
#include "net/alpha.hpp"

//TODO Split this file

#define likely(x)    __builtin_expect (!!(x), 1)

namespace {

using system_call = void(*)(interrupt::syscall_regs*);

std::array<system_call, 0x1000> system_calls;

int64_t expected_to_i64(const std::expected<void>& status){
    if(status){
        return 0;
    } else {
        return -status.error();
    }
}

int64_t expected_to_i64(const std::expected<size_t>& status){
    if(status){
        return *status;
    } else {
        return -status.error();
    }
}

void sc_log_string(interrupt::syscall_regs* regs){
    auto m = reinterpret_cast<const char*>(regs->rbx);
    logging::logf(logging::log_level::USER, "%s\n", m);
}

void sc_set_canonical(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    tty.set_canonical(regs->rbx);
}

void sc_set_mouse(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    tty.set_mouse(regs->rbx);
}

void sc_sleep_ms(interrupt::syscall_regs* regs){
    auto time = regs->rbx;

    scheduler::sleep_ms(time);
}

void sc_exec(interrupt::syscall_regs* regs){
    auto file = reinterpret_cast<char*>(regs->rbx);

    auto argc = regs->rcx;
    auto argv = reinterpret_cast<const char**>(regs->rdx);

    std::vector<std::string> params;

    for(size_t i = 0; i < argc; ++i){
        params.emplace_back(argv[i]);
    }

    auto status = scheduler::exec(file, params);
    regs->rax = expected_to_i64(status);
}

void sc_await_termination(interrupt::syscall_regs* regs){
    auto pid = regs->rbx;

    scheduler::await_termination(pid);
}

void sc_brk_start(interrupt::syscall_regs* regs){
    auto& process = scheduler::get_process(scheduler::get_pid());

    regs->rax = process.brk_start;
}

void sc_brk_end(interrupt::syscall_regs* regs){
    auto& process = scheduler::get_process(scheduler::get_pid());

    regs->rax = process.brk_end;
}

void sc_sbrk(interrupt::syscall_regs* regs){
    scheduler::sbrk(regs->rbx);

    auto& process = scheduler::get_process(scheduler::get_pid());
    regs->rax = process.brk_end;
}

void sc_get_columns(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    regs->rax = tty.get_console().get_columns();
}

void sc_get_rows(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    regs->rax = tty.get_console().get_rows();
}

void sc_clear_screen(interrupt::syscall_regs*){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    tty.get_console().wipeout();
}

void sc_reboot(interrupt::syscall_regs*){
    if(!acpi::initialized() || !acpi::reboot()){
        logging::logf(logging::log_level::ERROR, "ACPI reset not possible, fallback to 8042 reboot\n");
        asm volatile("mov al, 0x64; or al, 0xFE; out 0x64, al; mov al, 0xFE; out 0x64, al; " : : );
    }

    __builtin_unreachable();
}

void sc_shutdown(interrupt::syscall_regs*){
    if(!acpi::initialized()){
        logging::logf(logging::log_level::ERROR, "ACPI not initialized, impossible to shutdown\n");
        return;
    }

    acpi::shutdown();
}

void sc_open(interrupt::syscall_regs* regs){
    auto file = reinterpret_cast<char*>(regs->rbx);
    auto flags = regs->rcx;

    auto status = vfs::open(file, flags);
    regs->rax = expected_to_i64(status);
}

void sc_close(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;

    vfs::close(fd);
}

void sc_stat(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto info = reinterpret_cast<vfs::stat_info*>(regs->rcx);

    auto status = vfs::stat(fd, *info);
    regs->rax = expected_to_i64(status);
}

void sc_statfs(interrupt::syscall_regs* regs){
    auto mount_point  = reinterpret_cast<char*>(regs->rbx);
    auto info = reinterpret_cast<vfs::statfs_info*>(regs->rcx);

    auto status = vfs::statfs(mount_point, *info);
    regs->rax = expected_to_i64(status);
}

void sc_read(interrupt::syscall_regs* regs){
    auto fd     = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max    = regs->rdx;
    auto offset = regs->rsi;

    auto status = vfs::read(fd, buffer, max, offset);
    regs->rax = expected_to_i64(status);
}

void sc_read_timeout(interrupt::syscall_regs* regs){
    auto fd     = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max    = regs->rdx;
    auto offset = regs->rsi;
    auto ms     = regs->rdi;

    auto status = vfs::read(fd, buffer, max, offset, ms);
    regs->rax = expected_to_i64(status);
}

void sc_write(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max = regs->rdx;
    auto offset = regs->rsi;

    auto status = vfs::write(fd, buffer, max, offset);
    regs->rax = expected_to_i64(status);
}

void sc_clear(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto max = regs->rcx;
    auto offset = regs->rdx;

    auto status = vfs::clear(fd, max, offset);
    regs->rax = expected_to_i64(status);
}

void sc_truncate(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto size = regs->rcx;

    auto status = vfs::truncate(fd, size);
    regs->rax = expected_to_i64(status);
}

void sc_entries(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max = regs->rdx;

    auto status = vfs::entries(fd, buffer, max);
    regs->rax = expected_to_i64(status);
}

void sc_mounts(interrupt::syscall_regs* regs){
    auto buffer = reinterpret_cast<char*>(regs->rbx);
    auto max = regs->rcx;

    auto status = vfs::mounts(buffer, max);
    regs->rax = expected_to_i64(status);
}

void sc_mount(interrupt::syscall_regs* regs){
    auto type = static_cast<vfs::partition_type>(regs->rbx);
    auto mp_fd = regs->rcx;
    auto dev_fd = regs->rdx;

    auto status = vfs::mount(type, mp_fd, dev_fd);
    regs->rax = expected_to_i64(status);
}

void sc_pwd(interrupt::syscall_regs* regs){
    auto& wd = scheduler::get_working_directory();
    auto p = wd.string();

    auto buffer = reinterpret_cast<char*>(regs->rbx);
    std::copy(p.begin(), p.end(), buffer);
    buffer[p.size()] = '\0';
}

void sc_cwd(interrupt::syscall_regs* regs){
    auto p = reinterpret_cast<const char*>(regs->rbx);

    path cwd(p);
    scheduler::set_working_directory(cwd);
}

void sc_mkdir(interrupt::syscall_regs* regs){
    auto file = reinterpret_cast<char*>(regs->rbx);

    auto status = vfs::mkdir(file);
    regs->rax = expected_to_i64(status);
}

void sc_rm(interrupt::syscall_regs* regs){
    auto file = reinterpret_cast<char*>(regs->rbx);

    auto status = vfs::rm(file);
    regs->rax = expected_to_i64(status);
}

void sc_datetime(interrupt::syscall_regs* regs){
    auto date = reinterpret_cast<rtc::datetime*>(regs->rbx);

    *date = rtc::all_data();
}

void sc_time_seconds(interrupt::syscall_regs* regs){
    regs->rax = timer::seconds();
}

void sc_time_milliseconds(interrupt::syscall_regs* regs){
    regs->rax = timer::milliseconds();
}

void sc_vesa_width(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_width();
}
void sc_vesa_height(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_height();
}

void sc_vesa_shift_x(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_x_shift();
}

void sc_vesa_shift_y(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_y_shift();
}

void sc_vesa_bpsl(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_bytes_per_scan_line();
}

void sc_vesa_red_shift(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_red_shift();
}

void sc_vesa_green_shift(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_green_shift();
}

void sc_vesa_blue_shift(interrupt::syscall_regs* regs){
    regs->rax = vesa::get_blue_shift();
}

void sc_vesa_redraw(interrupt::syscall_regs* regs){
    auto new_buffer = reinterpret_cast<const char*>(regs->rbx);

    vesa::redraw(new_buffer);
}

void sc_mouse_x(interrupt::syscall_regs* regs){
    regs->rax = mouse::x();
}

void sc_mouse_y(interrupt::syscall_regs* regs){
    regs->rax = mouse::y();
}

void sc_ioctl(interrupt::syscall_regs* regs){
    auto device = regs->rbx;
    auto request = regs->rcx;
    auto data = reinterpret_cast<void*>(regs->rdx);

    auto status = ioctl(device, static_cast<io::ioctl_request>(request), data);
    regs->rax = expected_to_i64(status);
}

void sc_alpha(interrupt::syscall_regs*){
    network::alpha();
}

void sc_socket_open(interrupt::syscall_regs* regs){
    auto domain = static_cast<network::socket_domain>(regs->rbx);
    auto type = static_cast<network::socket_type>(regs->rcx);
    auto protocol = static_cast<network::socket_protocol>(regs->rdx);

    auto socket_fd = network::open(domain, type, protocol);

    if(socket_fd){
        regs->rax = *socket_fd;
    } else {
        regs->rax = -socket_fd.error();
    }
}

void sc_socket_close(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;

    network::close(fd);
}

void sc_prepare_packet(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto desc = reinterpret_cast<void*>(regs->rcx);
    auto buffer = reinterpret_cast<char*>(regs->rdx);

    int64_t fd;
    size_t index;
    std::tie(fd, index) = network::prepare_packet(socket_fd, desc, buffer);

    regs->rax = fd;
    regs->rbx = index;
}

void sc_finalize_packet(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto packet_fd = regs->rcx;

    auto status = network::finalize_packet(socket_fd, packet_fd);
    regs->rax = expected_to_i64(status);
}

void sc_send(interrupt::syscall_regs* regs){
    auto socket_fd     = regs->rbx;
    auto buffer        = reinterpret_cast<char*>(regs->rcx);
    auto n             = regs->rdx;
    auto target_buffer = reinterpret_cast<char*>(regs->rsi);

    regs->rax = expected_to_i64(network::send(socket_fd, buffer, n, target_buffer));
}

void sc_send_to(interrupt::syscall_regs* regs){
    auto socket_fd     = regs->rbx;
    auto buffer        = reinterpret_cast<char*>(regs->rcx);
    auto n             = regs->rdx;
    auto target_buffer = reinterpret_cast<char*>(regs->rsi);
    auto address       = reinterpret_cast<void*>(regs->rdi);

    regs->rax = expected_to_i64(network::send_to(socket_fd, buffer, n, target_buffer, address));
}

void sc_receive(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto buffer    = reinterpret_cast<char*>(regs->rcx);
    auto n         = regs->rdx;

    regs->rax = expected_to_i64(network::receive(socket_fd, buffer, n));
}

void sc_receive_timeout(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto buffer    = reinterpret_cast<char*>(regs->rcx);
    auto n         = regs->rdx;
    auto ms        = regs->rsi;

    regs->rax = expected_to_i64(network::receive(socket_fd, buffer, n, ms));
}

void sc_receive_from(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto buffer    = reinterpret_cast<char*>(regs->rcx);
    auto n         = regs->rdx;
    auto address   = reinterpret_cast<void*>(regs->rsi);

    regs->rax = expected_to_i64(network::receive_from(socket_fd, buffer, n, address));
}

void sc_receive_from_timeout(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto buffer    = reinterpret_cast<char*>(regs->rcx);
    auto n         = regs->rdx;
    auto ms        = regs->rsi;
    auto address   = reinterpret_cast<void*>(regs->rdi);

    regs->rax = expected_to_i64(network::receive_from(socket_fd, buffer, n, ms, address));
}

void sc_listen(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto listen = bool(regs->rcx);

    auto status = network::listen(socket_fd, listen);
    regs->rax = expected_to_i64(status);
}

void sc_client_bind(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto server_ip = regs->rcx;

    auto status = network::client_bind(socket_fd, server_ip);
    regs->rax = expected_to_i64(status);
}

void sc_client_bind_port(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto server_ip = regs->rcx;
    auto port      = regs->rdx;

    auto status = network::client_bind(socket_fd, server_ip, port);
    regs->rax = expected_to_i64(status);
}

void sc_server_bind(interrupt::syscall_regs* regs) {
    auto socket_fd = regs->rbx;
    auto local_ip  = regs->rcx;

    auto status = network::server_bind(socket_fd, local_ip);
    regs->rax   = expected_to_i64(status);
}

void sc_server_bind_port(interrupt::syscall_regs* regs) {
    auto socket_fd = regs->rbx;
    auto local_ip  = regs->rcx;
    auto port      = regs->rdx;

    auto status = network::server_bind(socket_fd, local_ip, port);
    regs->rax   = expected_to_i64(status);
}

void sc_client_unbind(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;

    auto status = network::client_unbind(socket_fd);
    regs->rax = expected_to_i64(status);
}

void sc_connect(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto ip = regs->rcx;
    auto port = regs->rdx;

    auto status = network::connect(socket_fd, ip, port);
    regs->rax = expected_to_i64(status);
}

void sc_server_start(interrupt::syscall_regs* regs) {
    auto socket_fd = regs->rbx;
    auto ip        = regs->rcx;
    auto port      = regs->rdx;

    auto status = network::server_start(socket_fd, ip, port);
    regs->rax   = expected_to_i64(status);
}

void sc_accept(interrupt::syscall_regs* regs) {
    auto socket_fd = regs->rbx;

    auto status = network::accept(socket_fd);
    regs->rax   = expected_to_i64(status);
}

void sc_accept_timeout(interrupt::syscall_regs* regs) {
    auto socket_fd = regs->rbx;
    auto ms        = regs->rcx;

    auto status = network::accept(socket_fd, ms);
    regs->rax   = expected_to_i64(status);
}

void sc_dns_server(interrupt::syscall_regs* regs){
    regs->rax = network::dns_server().raw_address;
}

void sc_disconnect(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;

    auto status = network::disconnect(socket_fd);
    regs->rax = expected_to_i64(status);
}

void sc_wait_for_packet(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto user_buffer = reinterpret_cast<char*>(regs->rcx);

    auto status = network::wait_for_packet(user_buffer, socket_fd);

    regs->rax = expected_to_i64(status);
    regs->rbx = reinterpret_cast<size_t>(user_buffer);
}

void sc_wait_for_packet_ms(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto user_buffer = reinterpret_cast<char*>(regs->rcx);
    auto ms = regs->rdx;

    auto status = network::wait_for_packet(user_buffer, socket_fd, ms);

    regs->rax = expected_to_i64(status);
    regs->rbx = reinterpret_cast<size_t>(user_buffer);
}

void sc_kill(interrupt::syscall_regs* /*regs*/) __attribute((noreturn));
void sc_kill(interrupt::syscall_regs* /*regs*/){
    scheduler::kill_current_process();
}

} //End of anonymous namespace

void system_call_entry(interrupt::syscall_regs* regs){
    auto code = regs->rax;

    if(likely(system_calls[code])){
        system_calls[code](regs);
        return;
    }

    logging::logf(logging::log_level::ERROR, "Invalid system call %h from %u\n", code, scheduler::get_pid());

    k_print_line("Invalid system call");

    scheduler::kill_current_process();
}

void install_system_calls(){
    if(!interrupt::register_syscall_handler(0, &system_call_entry)){
        logging::logf(logging::log_level::ERROR, "Unable to register syscall handler 0\n");
        return;
    }

    std::fill(system_calls.begin(), system_calls.end(), nullptr);

    system_calls[0x2] = sc_log_string;
    system_calls[0x4] = sc_sleep_ms;
    system_calls[0x5] = sc_exec;
    system_calls[0x6] = sc_await_termination;
    system_calls[0x7] = sc_brk_start;
    system_calls[0x8] = sc_brk_end;
    system_calls[0x9] = sc_sbrk;
    system_calls[0x20] = sc_set_canonical;
    system_calls[0x21] = sc_set_mouse;
    system_calls[0x22] = sc_clear_screen;
    system_calls[0x23] = sc_get_columns;
    system_calls[0x24] = sc_get_rows;
    system_calls[0x50] = sc_reboot;
    system_calls[0x51] = sc_shutdown;
    system_calls[0x300] = sc_open;
    system_calls[0x301] = sc_stat;
    system_calls[0x302] = sc_close;
    system_calls[0x303] = sc_read;
    system_calls[0x304] = sc_pwd;
    system_calls[0x305] = sc_cwd;
    system_calls[0x306] = sc_mkdir;
    system_calls[0x307] = sc_rm;
    system_calls[0x308] = sc_entries;
    system_calls[0x309] = sc_mounts;
    system_calls[0x310] = sc_statfs;
    system_calls[0x311] = sc_write;
    system_calls[0x312] = sc_truncate;
    system_calls[0x313] = sc_clear;
    system_calls[0x314] = sc_mount;
    system_calls[0x315] = sc_read_timeout;
    system_calls[0x400] = sc_datetime;
    system_calls[0x401] = sc_time_seconds;
    system_calls[0x402] = sc_time_milliseconds;
    system_calls[0x666] = sc_kill;
    system_calls[0xC00] = sc_vesa_width;
    system_calls[0xC01] = sc_vesa_height;
    system_calls[0xC02] = sc_vesa_shift_x;
    system_calls[0xC03] = sc_vesa_shift_y;
    system_calls[0xC04] = sc_vesa_bpsl;
    system_calls[0xC05] = sc_vesa_red_shift;
    system_calls[0xC06] = sc_vesa_green_shift;
    system_calls[0xC07] = sc_vesa_blue_shift;
    system_calls[0xC08] = sc_vesa_redraw;
    system_calls[0xCA0] = sc_mouse_x;
    system_calls[0xCA1] = sc_mouse_y;
    system_calls[0xA00] = sc_ioctl;
    system_calls[0xB00] = sc_socket_open;
    system_calls[0xB01] = sc_socket_close;
    system_calls[0xB02] = sc_prepare_packet;
    system_calls[0xB03] = sc_finalize_packet;
    system_calls[0xB04] = sc_listen;
    system_calls[0xB05] = sc_wait_for_packet;
    system_calls[0xB06] = sc_wait_for_packet_ms;
    system_calls[0xB07] = sc_client_bind;
    system_calls[0xB08] = sc_connect;
    system_calls[0xB09] = sc_disconnect;
    system_calls[0xB0A] = sc_client_unbind;
    system_calls[0xB0B] = sc_send;
    system_calls[0xB0C] = sc_receive_timeout;
    system_calls[0xB0D] = sc_client_bind_port;
    system_calls[0xB0E] = sc_server_bind;
    system_calls[0xB0F] = sc_server_bind_port;
    system_calls[0xB10] = sc_receive;
    system_calls[0xB11] = sc_receive_from;
    system_calls[0xB12] = sc_receive_from_timeout;
    system_calls[0xB12] = sc_receive_from_timeout;
    system_calls[0xB13] = sc_send_to;
    system_calls[0xB14] = sc_server_start;
    system_calls[0xB15] = sc_dns_server;
    system_calls[0xB16] = sc_accept;
    system_calls[0xB17] = sc_accept_timeout;
    system_calls[0x66] = sc_alpha;
}
