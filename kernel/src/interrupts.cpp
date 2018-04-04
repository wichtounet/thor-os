//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <types.hpp>

#include "interrupts.hpp"
#include "print.hpp"
#include "kernel_utils.hpp"
#include "gdt.hpp"
#include "scheduler.hpp"
#include "logging.hpp"

#include "isrs.hpp"
#include "irqs.hpp"
#include "syscalls.hpp"

namespace {

struct idt_flags {
    uint8_t type    : 4;
    uint8_t zero    : 1;
    uint8_t dpl     : 2;
    uint8_t present : 1;
} __attribute__((packed));

static_assert(sizeof(idt_flags) == 1, "The Flags of an IDT entry take one byte");

struct idt_entry {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  zero;
    idt_flags flags;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

static_assert(sizeof(idt_entry) == 16, "The size of an IDT entry should be 16 bytes");

struct idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

idt_entry idt_64[64];
idtr idtr_64;

void (*irq_handlers[16])(interrupt::syscall_regs*, void*);
void* irq_handler_data[16];
void (*syscall_handlers[interrupt::SYSCALL_MAX])(interrupt::syscall_regs*);

void idt_set_gate(size_t gate, void (*function)(void), uint16_t gdt_selector, idt_flags flags){
    auto& entry = idt_64[gate];

    entry.segment_selector = gdt_selector;
    entry.flags = flags;
    entry.reserved = 0;
    entry.zero = 0;

    auto function_address = reinterpret_cast<uintptr_t>(function);
    entry.offset_low = function_address & 0xFFFF;
    entry.offset_middle = (function_address >> 16) & 0xFFFF;
    entry.offset_high= function_address  >> 32;
}

uint64_t get_cr2(){
    uint64_t value;
    asm volatile("mov rax, cr2; mov %0, rax;" : "=m" (value));
    return value;
}

uint64_t get_cr3(){
    uint64_t value;
    asm volatile("mov rax, cr3; mov %0, rax;" : "=m" (value));
    return value;
}

void install_idt(){
    //Set the correct values inside IDTR
    idtr_64.limit = (64 * 16) - 1;
    idtr_64.base = reinterpret_cast<size_t>(&idt_64[0]);

    //Clear the IDT
    std::fill_n(reinterpret_cast<size_t*>(idt_64), 64 * sizeof(idt_entry) / sizeof(size_t), 0);

    //Clear the IRQ handlers
    std::fill_n(irq_handlers, 16, nullptr);
    std::fill_n(irq_handler_data, 16, nullptr);

    //Give the IDTR address to the CPU
    asm volatile("lidt [%0]" : : "m" (idtr_64));

    logging::logf(logging::log_level::TRACE, "int: IDT installed %h (base:%h)\n", reinterpret_cast<size_t>(&idtr_64), reinterpret_cast<size_t>(&idt_64[0]));
}

void install_isrs(){
    idt_set_gate(0, _isr0, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(1, _isr1, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(2, _isr2, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(3, _isr3, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(4, _isr4, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(5, _isr5, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(6, _isr6, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(7, _isr7, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(8, _isr8, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(9, _isr9, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(10, _isr10, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(11, _isr11, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(12, _isr12, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(13, _isr13, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(14, _isr14, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(15, _isr15, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(16, _isr16, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(17, _isr17, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(18, _isr18, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(19, _isr19, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(20, _isr20, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(21, _isr21, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(22, _isr22, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(23, _isr23, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(24, _isr24, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(25, _isr25, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(26, _isr26, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(27, _isr27, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(28, _isr28, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(29, _isr29, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(30, _isr30, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(31, _isr31, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
}

void remap_irqs(){
    //Restart the both PICs
    out_byte(0x20, 0x11);
    out_byte(0xA0, 0x11);

    out_byte(0x21, 0x20); //Make PIC1 start at 32
    out_byte(0xA1, 0x28); //Make PIC2 start at 40

    //Setup cascading for both PICs
    out_byte(0x21, 0x04);
    out_byte(0xA1, 0x02);

    //8086 mode for both PICs
    out_byte(0x21, 0x01);
    out_byte(0xA1, 0x01);

    //Activate all IRQs in both PICs
    out_byte(0x21, 0x0);
    out_byte(0xA1, 0x0);
}

void install_irqs(){
    idt_set_gate(32, _irq0, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(33, _irq1, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(34, _irq2, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(35, _irq3, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(36, _irq4, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(37, _irq5, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(38, _irq6, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(39, _irq7, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(40, _irq8, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(41, _irq9, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(42, _irq10, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(43, _irq11, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(44, _irq12, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(45, _irq13, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(46, _irq14, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(47, _irq15, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
}

void install_syscalls(){
    idt_set_gate(interrupt::SYSCALL_FIRST+0, _syscall0, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+1, _syscall1, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+2, _syscall2, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+3, _syscall3, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+4, _syscall4, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+5, _syscall5, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+6, _syscall6, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+7, _syscall7, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+8, _syscall8, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
    idt_set_gate(interrupt::SYSCALL_FIRST+9, _syscall9, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 3, 1});
}

void enable_interrupts(){
    asm volatile("sti" : : );
}

const char* exceptions_title[32] {
    "Division by zero",
    "Debugger",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bounds",
    "Invalid Opcode",
    "Coprocessor not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid Task State Segment",
    "Segment not present",
    "Stack Fault",
    "General protection fault",
    "Page Fault",
    "Reserved",
    "Math Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

} //end of anonymous namespace

extern "C" {

#define fault_printf(...) \
    logging::logf(logging::log_level::ERROR, __VA_ARGS__);

void _fault_handler(interrupt::fault_regs regs){
    fault_printf("Exception %u (%s) occured\n", regs.error_no, exceptions_title[regs.error_no]);
    fault_printf("error_code=%u\n", regs.error_code);
    fault_printf("rip=%h\n", regs.rip);
    fault_printf("rflags=%h\n", regs.rflags);
    fault_printf("cs=%h\n", regs.cs);
    fault_printf("rbp=%h\n", regs.rbp);
    fault_printf("rsp=%h\n", regs.rsp);
    fault_printf("ss=%h\n", regs.ss);
    fault_printf("pid=%u\n", scheduler::get_pid());
    fault_printf("cr2=%h\n", get_cr2());
    fault_printf("cr3=%h\n", get_cr3());

#ifdef THOR_STACK
    fault_printf("Call stack\n");
    size_t i = 0;
    auto rbp = regs.rbp;
    while(rbp){
        auto ip = *reinterpret_cast<size_t*>(rbp + 8);
        fault_printf("%u: %h\n", i++, ip);
        rbp = *reinterpret_cast<size_t*>(rbp);
    }
#endif

    // TODO Should also print the message to the terminal of the process
    // (cannot use printf because of string manipulation)

    if(scheduler::is_started()){
        // TODO Should also send a signal to the process (if user)
        scheduler::fault();
    } else {
        asm volatile ("cli; hlt;");
    }
}

void _irq_handler(interrupt::syscall_regs* regs){
    //If the IRQ is on the slave controller, send EOI to it
    if(regs->code >= 8){
        out_byte(0xA0, 0x20);
    }

    //Send EOI to the master controller
    out_byte(0x20, 0x20);

    //If there is an handler, call it
    if(irq_handlers[regs->code]){
        irq_handlers[regs->code](regs, irq_handler_data[regs->code]);
    }
}

void _syscall_handler(interrupt::syscall_regs* regs){
    //If there is a handler call it
    if(syscall_handlers[regs->code]){
        syscall_handlers[regs->code](regs);
    }

    //TODO Emit an error somehow if there is no handler
}

} //end of extern "C"

bool interrupt::register_irq_handler(size_t irq, void (*handler)(interrupt::syscall_regs*, void*), void* data){
    if(irq_handlers[irq]){
        logging::logf(logging::log_level::ERROR, "Register interrupt %u while already registered\n", irq);
        return false;
    }

    if(irq > 15){
        logging::logf(logging::log_level::ERROR, "Register interrupt %u too high\n", irq);
        return false;
    }

    irq_handlers[irq] = handler;
    irq_handler_data[irq] = data;

    return true;
}

bool interrupt::register_syscall_handler(size_t syscall, void (*handler)(interrupt::syscall_regs*)){
    if(syscall_handlers[syscall]){
        logging::logf(logging::log_level::ERROR, "Register syscall %u while already registered\n", syscall);
        return false;
    }

    if(syscall > interrupt::SYSCALL_MAX){
        logging::logf(logging::log_level::ERROR, "Register syscall %u too high\n", syscall);
        return false;
    }

    syscall_handlers[syscall] = handler;

    return true;
}

bool interrupt::unregister_irq_handler(size_t irq, void (*handler)(interrupt::syscall_regs*, void*)){
    if(!irq_handlers[irq]){
        logging::logf(logging::log_level::ERROR, "Unregister interrupt %u while not registered\n", irq);
        return false;
    }

    if(irq > 15){
        logging::logf(logging::log_level::ERROR, "Unregister interrupt %u too high\n", irq);
        return false;
    }

    if(irq_handlers[irq] != handler){
        logging::logf(logging::log_level::ERROR, "Unregister wrong irq handler %u\n", irq);
        return false;
    }


    irq_handlers[irq] = nullptr;
    irq_handler_data[irq] = nullptr;

    return true;
}

bool interrupt::unregister_syscall_handler(size_t syscall, void (*handler)(interrupt::syscall_regs*)){
    if(!syscall_handlers[syscall]){
        logging::logf(logging::log_level::ERROR, "Unregister syscall %u while not registered\n", syscall);
        return false;
    }

    if(syscall > interrupt::SYSCALL_MAX){
        logging::logf(logging::log_level::ERROR, "Unregister syscall %u too high\n", syscall);
        return false;
    }

    if(syscall_handlers[syscall] != handler){
        logging::logf(logging::log_level::ERROR, "Unregister wrong syscall handler %u\n", syscall);
        return false;
    }

    syscall_handlers[syscall] = nullptr;

    return true;
}

void interrupt::setup_interrupts(){
    install_idt();
    install_isrs();
    remap_irqs();
    install_irqs();
    install_syscalls();
    enable_interrupts();
}
