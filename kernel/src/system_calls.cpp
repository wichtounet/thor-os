//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "system_calls.hpp"
#include "console.hpp"
#include "scheduler.hpp"
#include "keyboard.hpp"
#include "terminal.hpp"
#include "acpi.hpp"
#include "vfs.hpp"

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

    regs->rax = tty.read_input(reinterpret_cast<char*>(regs->rbx), regs->rcx);
}

void sc_sleep_ms(interrupt::syscall_regs* regs){
    auto time = regs->rbx;

    scheduler::sleep_ms(scheduler::get_pid(), time);
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

void sc_clear(interrupt::syscall_regs*){
    wipeout();
}

void sc_reboot(interrupt::syscall_regs*){
    //TODO Reboot should be done more properly
    asm volatile("mov al, 0x64; or al, 0xFE; out 0x64, al; mov al, 0xFE; out 0x64, al; " : : );

    __builtin_unreachable();
}

void sc_shutdown(interrupt::syscall_regs*){
    if(!acpi::init()){
        k_print_line("Unable to init ACPI");
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

void sc_read(interrupt::syscall_regs* regs){
    auto fd = regs->rbx;
    auto buffer = reinterpret_cast<char*>(regs->rcx);
    auto max = regs->rdx;

    regs->rax = vfs::read(fd, buffer, max);
}

void sc_pwd(interrupt::syscall_regs* regs){
    auto& wd = scheduler::get_working_directory();

    std::string path;
    path += '/';

    for(auto& part : wd){
        path += part;
        path += '/';
    }

    auto buffer = reinterpret_cast<char*>(regs->rbx);
    for(int i = 0; i < path.size(); ++i){
        buffer[i] = path[i];
    }
    buffer[path.size()] = '\0';
}

void sc_cwd(interrupt::syscall_regs* regs){
    auto p = reinterpret_cast<const char*>(regs->rbx);
    std::string path(p);

    auto cwd = std::split(path, '/');
    scheduler::set_working_directory(cwd);
}

void sc_mkdir(interrupt::syscall_regs* regs){
    auto file = reinterpret_cast<char*>(regs->rbx);

    regs->rax = vfs::mkdir(file);
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

        case 3:
            sc_get_input(regs);
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

        case 100:
            sc_clear(regs);
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

        case 0x666:
            //TODO Do something with return code
            scheduler::kill_current_process();
            break;

        default:
            k_print_line("Invalid system call");
            break;
    }
}

void install_system_calls(){
    interrupt::register_syscall_handler(0, &system_call_entry);
}
