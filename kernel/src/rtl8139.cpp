//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "rtl8139.hpp"
#include "logging.hpp"
#include "kernel_utils.hpp"
#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "interrupts.hpp"
#include "paging.hpp"

#define MAC0 0x00
#define MAC4 0x04
#define RBSTART 0x30
#define CMD 0x37
#define IMR 0x3C //Interrupt mask register
#define ISR 0x3E //Interrupt status register
#define RCR 0x44 //Receive Config Register
#define CONFIG_1 0x52
#define RX_BUF_PTR 0x38
#define RX_BUF_ADDR 0x3A

#define RCR_AAP  (1 << 0) /* Accept All Packets */
#define RCR_APM  (1 << 1) /* Accept Physical Match Packets */
#define RCR_AM   (1 << 2) /* Accept Multicast Packets */
#define RCR_AB   (1 << 3) /* Accept Broadcast Packets */
#define RCR_WRAP (1 << 7) /* Wrap packets too long */

namespace {

struct rtl8139_t {
    uint32_t iobase;
    uint64_t phys_buffer_rx;
    uint64_t buffer_rx;

    uint64_t cur_rx; //Index inside the buffer
};

//TODO Add a way so that the interrupt handler is able to pass a void ptr to the handler

volatile rtl8139_t* saved_desc;

void packet_handler(interrupt::syscall_regs*){
    logging::logf(logging::log_level::TRACE, "rtl8139: Packet Received\n");

    auto& desc = *saved_desc;

    //This should be a packet header
    uint16_t header = *reinterpret_cast<uint16_t*>(desc.buffer_rx);

    logging::logf(logging::log_level::TRACE, "rtl8139: Packet Received %u\n", uint64_t(header));

    // Get the interrupt status
    auto status = in_word(desc.iobase + ISR);

    // Acknowledge the handling of the packet
    out_word(desc.iobase + ISR, status);

#define RX_OK 0x01
#define CMD_NOT_EMPTY 0x01

    if(status & RX_OK){
        logging::logf(logging::log_level::TRACE, "rtl8139: Receive status OK\n");

        auto cur_rx = desc.cur_rx;
        auto buffer_rx = desc.buffer_rx;

        while((in_byte(desc.iobase + CMD) & CMD_NOT_EMPTY) == 0){
            auto packet_header = *reinterpret_cast<uint16_t*>(buffer_rx + cur_rx);
            auto packet_length = *reinterpret_cast<uint16_t*>(buffer_rx + cur_rx + 2);
            auto packet_only_length = packet_length - 4;

            logging::logf(logging::log_level::TRACE, "rtl8139: Handle Packet Received %u\n", uint64_t(packet_header));
            logging::logf(logging::log_level::TRACE, "rtl8139: Handle Packet Received %u\n", uint64_t(packet_length));

            cur_rx = (cur_rx + packet_length + 4 + 3) & ~3;
            out_word(desc.iobase + RX_BUF_PTR, cur_rx - 16);
        }

        logging::logf(logging::log_level::TRACE, "rtl8139: Packet Handled\n");

        desc.cur_rx = cur_rx;
    } else {
        logging::logf(logging::log_level::TRACE, "rtl8139: Receive status not OK\n");
    }
}

} //end of anonymous namespace

void rtl8139::init_driver(network::interface_descriptor& interface, pci::device_descriptor& pci_device){
    logging::logf(logging::log_level::TRACE, "rtl8139: Initialize RTL8139 driver on pci:%u:%u:%u\n", uint64_t(pci_device.bus), uint64_t(pci_device.device), uint64_t(pci_device.function));

    rtl8139_t* desc = new rtl8139_t();
    interface.driver_data = desc;
    saved_desc = desc;

    // 1. Enable PCI Bus Mastering (allows DMA)

    auto command_register = pci::read_config_dword(pci_device.bus, pci_device.device, pci_device.function, 4);
    command_register |= 2; // Set Bus Mastering Bit
    pci::write_config_dword(pci_device.bus, pci_device.device, pci_device.function, 4, command_register);

    // 2. Get the I/O base address

    auto iobase = pci::read_config_dword(pci_device.bus, pci_device.device, pci_device.function, 0x10) & (~0x3);
    desc->iobase = iobase;

    logging::logf(logging::log_level::TRACE, "rtl8139: I/O Base address :%u\n", uint64_t(iobase));

    // 3. Power on the device

    out_byte(iobase + CONFIG_1, 0x0);

    // 4. Software reset

    out_byte(iobase + CMD, 0x10);
    while( (in_byte(iobase + CMD) & 0x10) != 0) { /* Wait for RST to be done */ }

    // 5. Init the receive buffer

    auto buffer_rx_phys = physical_allocator::allocate(3);
    out_dword(iobase + RBSTART, buffer_rx_phys);

    auto buffer_rx_virt = virtual_allocator::allocate(3);
    if(!paging::map_pages(buffer_rx_virt, buffer_rx_phys, 3)){
        logging::logf(logging::log_level::ERROR, "rtl8139: I/O Unable to map %u into %u\n", buffer_rx_phys, buffer_rx_virt);
    }

    desc->phys_buffer_rx = buffer_rx_phys;
    desc->buffer_rx = buffer_rx_virt;
    desc->cur_rx = 0;

    // 6. Register IRQ handler

    auto irq = pci::read_config_dword(pci_device.bus, pci_device.device, pci_device.function, 0x3c) & 0xFF;
    interrupt::register_irq_handler(irq, packet_handler);

    // 7. Set IMR + ISR

    logging::logf(logging::log_level::TRACE, "rtl8139: IRQ :%u\n", uint64_t(irq));

    out_word(iobase + IMR, 0x0005); // Sets the TOK and ROK bits high

    // 8. Set RCR (Receive Configuration Register)

    out_dword(iobase + RCR, RCR_AAP | RCR_APM | RCR_AM | RCR_AB | RCR_WRAP);

    // 9. Enable RX and TX

    out_byte(iobase + CMD, 0x0C); // Sets the RE and TE bits high

    // 10. Get the mac address

    size_t mac = 0;

    for(size_t i = 0; i < 6; ++i){
        mac |= uint64_t(in_byte(iobase + i)) << ((5 - i) * 8);
    }

    interface.mac_address = mac;

    logging::logf(logging::log_level::TRACE, "rtl8139: MAC Address %h n", mac);
}
