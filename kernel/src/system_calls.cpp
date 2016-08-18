//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "system_calls.hpp"
#include "console.hpp"
#include "scheduler.hpp"
#include "timer.hpp"
#include "keyboard.hpp"
#include "terminal.hpp"
#include "acpi.hpp"
#include "rtc.hpp"
#include "kernel_utils.hpp"
#include "vesa.hpp"
#include "mouse.hpp"
#include "vfs/vfs.hpp"
#include "ioctl.hpp"

//TODO Split this file

namespace {

void sc_print_char(interrupt::syscall_regs* regs){
    k_print(static_cast<char>(regs->rbx));
}

void sc_print_string(interrupt::syscall_regs* regs){
    k_print(reinterpret_cast<const char*>(regs->rbx));
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

void sc_set_canonical(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    tty.set_canonical(regs->rbx);
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

    regs->rax = scheduler::exec(file, params);
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

    regs->rax = vfs::open(file, flags);
}

void sc_close(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;

    vfs::close(fd);
}

void sc_stat(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto info = reinterpret_cast<stat_info*>(regs->rcx);

    regs->rax = vfs::stat(fd, *info);
}

void sc_statfs(interrupt::syscall_regs* regs){
    auto mount_point  = reinterpret_cast<char*>(regs->rbx);
    auto info = reinterpret_cast<statfs_info*>(regs->rcx);

    regs->rax = vfs::statfs(mount_point, *info);
}

void sc_read(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max = regs->rdx;
    auto offset = regs->rsi;

    regs->rax = vfs::read(fd, buffer, max, offset);
}

void sc_write(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max = regs->rdx;
    auto offset = regs->rsi;

    regs->rax = vfs::write(fd, buffer, max, offset);
}

void sc_clear(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto max = regs->rcx;
    auto offset = regs->rdx;

    regs->rax = vfs::clear(fd, max, offset);
}

void sc_truncate(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto size = regs->rcx;

    regs->rax = vfs::truncate(fd, size);
}

void sc_entries(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max = regs->rdx;

    regs->rax = vfs::entries(fd, buffer, max);
}

void sc_mounts(interrupt::syscall_regs* regs){
    auto buffer = reinterpret_cast<char*>(regs->rbx);
    auto max = regs->rcx;

    regs->rax = vfs::mounts(buffer, max);
}

void sc_mount(interrupt::syscall_regs* regs){
    auto type = static_cast<vfs::partition_type>(regs->rbx);
    auto mp_fd = regs->rcx;
    auto dev_fd = regs->rdx;

    regs->rax = vfs::mount(type, mp_fd, dev_fd);
}

void sc_pwd(interrupt::syscall_regs* regs){
    auto& wd = scheduler::get_working_directory();
    auto p = wd.string();

    auto buffer = reinterpret_cast<char*>(regs->rbx);
    for(size_t i = 0; i < p.size(); ++i){
        buffer[i] = p[i];
    }
    buffer[p.size()] = '\0';
}

void sc_cwd(interrupt::syscall_regs* regs){
    auto p = reinterpret_cast<const char*>(regs->rbx);

    path cwd(p);
    scheduler::set_working_directory(cwd);
}

void sc_mkdir(interrupt::syscall_regs* regs){
    auto file = reinterpret_cast<char*>(regs->rbx);

    regs->rax = vfs::mkdir(file);
}

void sc_rm(interrupt::syscall_regs* regs){
    auto file = reinterpret_cast<char*>(regs->rbx);

    regs->rax = vfs::rm(file);
}

void sc_datetime(interrupt::syscall_regs* regs){
    auto date = reinterpret_cast<datetime*>(regs->rbx);

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

    regs->rax = ioctl(device, static_cast<ioctl_request>(request), data);
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

        case 0x2000:
            sc_ioctl(regs);
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
