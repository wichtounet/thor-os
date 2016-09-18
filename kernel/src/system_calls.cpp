//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "system_calls.hpp"
#include "console.hpp"
#include "scheduler.hpp"
#include "timer.hpp"
#include "drivers/keyboard.hpp"
#include "terminal.hpp"
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

namespace {

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

void sc_print_char(interrupt::syscall_regs* regs){
    k_print(static_cast<char>(regs->rbx));
}

void sc_print_string(interrupt::syscall_regs* regs){
    k_print(reinterpret_cast<const char*>(regs->rbx));
}

void sc_log_string(interrupt::syscall_regs* regs){
    auto m = reinterpret_cast<const char*>(regs->rbx);
    logging::logf(logging::log_level::USER, "%s\n", m);
}

void sc_get_input(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    regs->rax = tty.read_input_can(reinterpret_cast<char*>(regs->rbx), regs->rcx);
}

void sc_get_input_timeout(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    regs->rax = tty.read_input_can(reinterpret_cast<char*>(regs->rbx), regs->rcx, regs->rdx);
}

void sc_get_input_raw(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    regs->rax = tty.read_input_raw();
}

void sc_get_input_raw_timeout(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    regs->rax = tty.read_input_raw(regs->rbx);
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
    regs->rax = get_columns();
}

void sc_get_rows(interrupt::syscall_regs* regs){
    regs->rax = get_rows();
}

void sc_clear_screen(interrupt::syscall_regs*){
    wipeout();
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
    auto fd = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max = regs->rdx;
    auto offset = regs->rsi;

    auto status = vfs::read(fd, buffer, max, offset);
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

void sc_connect(interrupt::syscall_regs* regs){
    auto socket_fd = regs->rbx;
    auto ip = regs->rcx;
    auto port = regs->rdx;

    auto status = network::connect(socket_fd, ip, port);
    regs->rax = expected_to_i64(status);
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

} //End of anonymous namespace

void system_call_entry(interrupt::syscall_regs* regs){
    auto code = regs->rax;

    switch(code){
        case 0:
            sc_print_char(regs);
            break;

        case 1:
            sc_print_string(regs);
            break;

        case 2:
            sc_log_string(regs);
            break;

        case 4:
            sc_sleep_ms(regs);
            break;

        case 5:
            sc_exec(regs);
            break;

        case 6:
            sc_await_termination(regs);
            break;

        case 7:
            sc_brk_start(regs);
            break;

        case 8:
            sc_brk_end(regs);
            break;

        case 9:
            sc_sbrk(regs);
            break;

        case 0x10:
            sc_get_input(regs);
            break;

        case 0x11:
            sc_get_input_timeout(regs);
            break;

        case 0x12:
            sc_get_input_raw(regs);
            break;

        case 0x13:
            sc_get_input_raw_timeout(regs);
            break;

        case 0x20:
            sc_set_canonical(regs);
            break;

        case 0x21:
            sc_set_mouse(regs);
            break;

        case 100:
            sc_clear_screen(regs);
            break;

        case 101:
            sc_get_columns(regs);
            break;

        case 102:
            sc_get_rows(regs);
            break;

        case 201:
            sc_reboot(regs);
            break;

        case 202:
            sc_shutdown(regs);
            break;

        case 300:
            sc_open(regs);
            break;

        case 301:
            sc_stat(regs);
            break;

        case 302:
            sc_close(regs);
            break;

        case 303:
            sc_read(regs);
            break;

        case 304:
            sc_pwd(regs);
            break;

        case 305:
            sc_cwd(regs);
            break;

        case 306:
            sc_mkdir(regs);
            break;

        case 307:
            sc_rm(regs);
            break;

        case 308:
            sc_entries(regs);
            break;

        case 309:
            sc_mounts(regs);
            break;

        case 310:
            sc_statfs(regs);
            break;

        case 311:
            sc_write(regs);
            break;

        case 312:
            sc_truncate(regs);
            break;

        case 313:
            sc_clear(regs);
            break;

        case 314:
            sc_mount(regs);
            break;

        case 0x400:
            sc_datetime(regs);
            break;

        case 0x401:
            sc_time_seconds(regs);
            break;

        case 0x402:
            sc_time_milliseconds(regs);
            break;

        case 0x666:
            //TODO Do something with return code
            scheduler::kill_current_process();
            break;

        case 0x1000:
            sc_vesa_width(regs);
            break;

        case 0x1001:
            sc_vesa_height(regs);
            break;

        case 0x1002:
            sc_vesa_shift_x(regs);
            break;

        case 0x1003:
            sc_vesa_shift_y(regs);
            break;

        case 0x1004:
            sc_vesa_bpsl(regs);
            break;

        case 0x1005:
            sc_vesa_red_shift(regs);
            break;

        case 0x1006:
            sc_vesa_green_shift(regs);
            break;

        case 0x1007:
            sc_vesa_blue_shift(regs);
            break;

        case 0x1008:
            sc_vesa_redraw(regs);
            break;

        case 0x1100:
            sc_mouse_x(regs);
            break;

        case 0x1101:
            sc_mouse_y(regs);
            break;

        // I/O system calls

        case 0x2000:
            sc_ioctl(regs);
            break;

        // Network system calls

        case 0x3000:
            sc_socket_open(regs);
            break;

        case 0x3001:
            sc_socket_close(regs);
            break;

        case 0x3002:
            sc_prepare_packet(regs);
            break;

        case 0x3003:
            sc_finalize_packet(regs);
            break;

        case 0x3004:
            sc_listen(regs);
            break;

        case 0x3005:
            sc_wait_for_packet(regs);
            break;

        case 0x3006:
            sc_wait_for_packet_ms(regs);
            break;

        case 0x3007:
            sc_client_bind(regs);
            break;

        case 0x3008:
            sc_connect(regs);
            break;

        case 0x3009:
            sc_disconnect(regs);
            break;

        // Special system calls

        case 0x6666:
            sc_alpha(regs);
            break;

        default:
            k_print_line("Invalid system call");
            break;
    }
}

void install_system_calls(){
    if(!interrupt::register_syscall_handler(0, &system_call_entry)){
        logging::logf(logging::log_level::ERROR, "Unable to register syscall handler 0\n");
    }
}
