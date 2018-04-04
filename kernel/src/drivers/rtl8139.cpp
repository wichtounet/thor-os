//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "drivers/rtl8139.hpp"

#include "conc/mutex.hpp"
#include "conc/int_lock.hpp"
#include "conc/int_lock.hpp"

#include "net/ethernet_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"
#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "interrupts.hpp"
#include "paging.hpp"

#define MAC0 0x00
#define MAC4 0x04
#define CMD 0x37
#define IMR 0x3C //Interrupt mask register
#define ISR 0x3E //Interrupt status register
#define RCR 0x44 //Receive Config Register
#define CONFIG_1 0x52
#define RX_BUF 0x30
#define RX_BUF_PTR 0x38
#define RX_BUF_ADDR 0x3A

#define TX_STATUS 0x10 // First TX status register (32 bit)
#define TX_ADDR   0x20 // First Tx Address register (32 bit)

#define RX_MISSED 0x4C
#define CMD_NOT_EMPTY 0x01

#define RX_OK 0x01 // Receive OK
#define TX_OK 0x04 // Transmit OK
#define TX_ERR 0x08 // Transmit error

#define RCR_AAP  (1 << 0) /* Accept All Packets */
#define RCR_APM  (1 << 1) /* Accept Physical Match Packets */
#define RCR_AM   (1 << 2) /* Accept Multicast Packets */
#define RCR_AB   (1 << 3) /* Accept Broadcast Packets */
#define RCR_WRAP (1 << 7) /* Wrap packets too long */

#define RX_STATUS_OK 0x1
#define RX_BAD_ALIGN 0x2
#define RX_CRC_ERR 0x4
#define RX_TOO_LONG 0x8
#define RX_RUNT 0x10
#define RX_BAD_SYMBOL 0x20
#define RX_BROADCAST 0x2000
#define RX_PHYSICAL 0x4000
#define RX_MULTICAST 0x8000

#define TX_STATUS_HOST_OWNS 0x00002000
#define TX_STATUS_UNDERRUN 0x00004000
#define TX_STATUS_OK 0x00008000
#define TX_STATUS_OUT_OF_WINDOW 0x20000000
#define TX_STATUS_ABORTED 0x40000000
#define TX_STATUS_CARRIER_LOST 0x80000000

namespace {

constexpr const size_t tx_buffers = 4;
constexpr const size_t tx_buffer_size = 0x2000;

struct tx_desc_t {
    size_t buffer_phys;
    size_t buffer_virt;
};

struct rtl8139_t {
    uint32_t iobase;
    uint64_t phys_buffer_rx;
    uint64_t buffer_rx;

    uint64_t cur_rx; //Index inside the receive buffer
    volatile uint64_t cur_tx; //Index inside the transmit buffer
    volatile uint64_t dirty_tx; //Index inside the transmit buffer

    tx_desc_t tx_desc[tx_buffers];

    mutex tx_lock;
    deferred_unique_semaphore tx_sem;

    network::interface_descriptor* interface;
};

void packet_handler(interrupt::syscall_regs*, void* data){
    auto& desc = *static_cast<rtl8139_t*>(data);
    auto& interface = *desc.interface;

    // Get the interrupt status
    auto status = in_word(desc.iobase + ISR);

    // Acknowledge the handling of the packet
    out_word(desc.iobase + ISR, status);

    if(status & RX_OK){
        logging::logf(logging::log_level::TRACE, "rtl8139: Packet received correctly OK\n");

        auto cur_rx = desc.cur_rx;

        while((in_byte(desc.iobase + CMD) & CMD_NOT_EMPTY) == 0){
            auto cur_offset = cur_rx % 0x3000;
            auto buffer_rx = reinterpret_cast<char*>(desc.buffer_rx);

            auto packet_status = *reinterpret_cast<uint32_t*>(buffer_rx + cur_offset);
            auto packet_length = packet_status >> 16; //Extract the size from the header
            auto packet_payload = buffer_rx + cur_offset + 4; //Skip the packet header (NIC)

            if (packet_status & (RX_BAD_SYMBOL | RX_RUNT | RX_TOO_LONG | RX_CRC_ERR | RX_BAD_ALIGN)) {
                logging::logf(logging::log_level::TRACE, "rtl8139: Packet Error, status:%u\n", uint64_t(packet_status));

                //TODO We should probably reset the controller ?
            } else if(packet_length == 0){
                // TODO Normally this should not happen, it probably indicates a bug somewhere
                logging::logf(logging::log_level::TRACE, "rtl8139: Packet Error Length = 0, status:%u\n", uint64_t(packet_status));
            } else {
                // Omit CRC from the length
                auto packet_only_length = packet_length - 4;

                logging::logf(logging::log_level::TRACE, "rtl8139: Packet OK length:%u\n", uint64_t(packet_only_length));

                auto packet_buffer = new char[packet_only_length];

                std::copy_n(packet_payload, packet_only_length, packet_buffer);

                interface.rx_queue.emplace(std::make_shared<network::packet>(packet_buffer, packet_only_length));
                interface.rx_sem.notify();
            }

            cur_rx = (cur_rx + packet_length + 4 + 3) & ~3; //align on 4 bytes
            out_word(desc.iobase + RX_BUF_PTR, cur_rx - 0x10);

            logging::logf(logging::log_level::TRACE, "rtl8139: Packet Handled\n");
        }

        desc.cur_rx = cur_rx;
    }

    if(status & (TX_OK | TX_ERR)){
        auto& dirty_tx = desc.dirty_tx;
        size_t cleaned_up = 0;

        while(desc.cur_tx - dirty_tx > 0){
            auto entry = dirty_tx % tx_buffers;

            auto tx_status = in_dword(desc.iobase + TX_STATUS + entry * 4);

            // Check if the packet has already been transmitted
            if(!(tx_status & (TX_STATUS_OK | TX_STATUS_ABORTED | TX_STATUS_UNDERRUN))){
                break;
            }

            if(tx_status & (TX_STATUS_OUT_OF_WINDOW | TX_STATUS_ABORTED)){
                if(tx_status & TX_STATUS_CARRIER_LOST){
                    logging::logf(logging::log_level::ERROR, "rtl8139: Carrier lost\n");
                } else if (tx_status & TX_STATUS_OUT_OF_WINDOW){
                    logging::logf(logging::log_level::ERROR, "rtl8139: Out of window\n");
                } else {
                    logging::logf(logging::log_level::TRACE, "rtl8139: Packet abortd\n");
                }
            } else {
                logging::logf(logging::log_level::TRACE, "rtl8139: Packet transmitted correctly\n");
            }

            ++cleaned_up;
            ++dirty_tx;
        }

        desc.tx_sem.notify(cleaned_up);
    }

    if(!(status & (RX_OK | TX_OK | TX_ERR))){
        // This should not happen since we only enable a few
        // interrupts
        logging::logf(logging::log_level::ERROR, "rtl8139: Receive status unhandled OK\n");
    }
}

void send_packet(network::interface_descriptor& interface, network::packet_p& packet){
    logging::logf(logging::log_level::TRACE, "rtl8139: Start transmitting packet (%p)\n", packet.get());

    auto* ether_header = reinterpret_cast<network::ethernet::header*>(packet->payload);

    // Shortcut packet to self directly to the rx queue
    if(network::ethernet::mac6_to_mac64(ether_header->target.mac) == interface.mac_address){
        {
            direct_int_lock lock;

            interface.rx_queue.push(packet);
            interface.rx_sem.notify();
        }

        logging::logf(logging::log_level::TRACE, "rtl8139: Packet to self transmitted correctly\n");

        return;
    }

    auto& desc = *reinterpret_cast<rtl8139_t*>(interface.driver_data);
    auto iobase = desc.iobase;

    desc.tx_lock.lock();

    // Wait for a free entry in the tx buffers
    desc.tx_sem.claim();
    desc.tx_sem.wait();

    // Claim an entry in the tx buffers
    auto entry = desc.cur_tx++ % tx_buffers;

    desc.tx_lock.unlock();

    auto& tx_desc = desc.tx_desc[entry];

    std::copy_n(packet->payload, packet->payload_size, reinterpret_cast<char*>(tx_desc.buffer_virt));

    out_dword(iobase + TX_ADDR + entry * 4, tx_desc.buffer_phys);
    out_dword(iobase + TX_STATUS + entry * 4, uint32_t(256) << 16 | packet->payload_size);
}

} //end of anonymous namespace

void rtl8139::init_driver(network::interface_descriptor& interface, pci::device_descriptor& pci_device){
    logging::logf(logging::log_level::TRACE, "rtl8139: Initialize RTL8139 driver on pci:%u:%u:%u\n", uint64_t(pci_device.bus), uint64_t(pci_device.device), uint64_t(pci_device.function));

    rtl8139_t* desc = new rtl8139_t();

    interface.driver_data = desc;
    interface.hw_send = send_packet;

    desc->tx_sem.init(tx_buffers);

    for(size_t i = 0; i < tx_buffers; ++i){
        auto& tx_desc = desc->tx_desc[i];

        tx_desc.buffer_phys = physical_allocator::allocate(2);
        tx_desc.buffer_virt = virtual_allocator::allocate(2);

        if(!paging::map_pages(tx_desc.buffer_virt, tx_desc.buffer_phys, 2)){
            logging::logf(logging::log_level::ERROR, "rtl8139: Unable to map %h into %h\n", tx_desc.buffer_virt, tx_desc.buffer_phys);
        }
    }

    // 1. Enable PCI Bus Mastering (allows DMA)

    auto command_register = pci::read_config_dword(pci_device.bus, pci_device.device, pci_device.function, 0x4);
    command_register |= 0x4; // Set Bus Mastering Bit
    pci::write_config_dword(pci_device.bus, pci_device.device, pci_device.function, 0x4, command_register);

    // 2. Get the I/O base address

    auto iobase = pci::read_config_dword(pci_device.bus, pci_device.device, pci_device.function, 0x10) & (~0x3);
    desc->iobase = iobase;

    logging::logf(logging::log_level::TRACE, "rtl8139: I/O Base address :%h\n", uint64_t(iobase));

    // 3. Power on the device

    out_byte(iobase + CONFIG_1, 0x0);

    // 4. Software reset

    out_byte(iobase + CMD, 0x10);
    while( (in_byte(iobase + CMD) & 0x10) != 0) { /* Wait for RST to be done */ }

    // 5. Init the receive buffer

    auto buffer_rx_phys = physical_allocator::allocate(3);
    out_dword(iobase + RX_BUF, buffer_rx_phys);
    out_dword(iobase + RX_BUF_PTR, 0);
    out_dword(iobase + RX_BUF_ADDR, 0);

    auto buffer_rx_virt = virtual_allocator::allocate(3);
    if(!paging::map_pages(buffer_rx_virt, buffer_rx_phys, 3)){
        logging::logf(logging::log_level::ERROR, "rtl8139: Unable to map %h into %h\n", buffer_rx_phys, buffer_rx_virt);
    }

    desc->phys_buffer_rx = buffer_rx_phys;
    desc->buffer_rx = buffer_rx_virt;
    desc->cur_rx = 0;
    desc->cur_tx = 0;
    desc->dirty_tx = 0;

    std::fill_n(reinterpret_cast<char*>(desc->buffer_rx), 0x3000, 0);

    logging::logf(logging::log_level::TRACE, "rtl8139: Physical RX Buffer :%h\n", uint64_t(desc->phys_buffer_rx));
    logging::logf(logging::log_level::TRACE, "rtl8139: Virtual RX Buffer :%h\n", uint64_t(desc->buffer_rx));

    // 6. Register IRQ handler

    auto irq = pci::read_config_dword(pci_device.bus, pci_device.device, pci_device.function, 0x3c) & 0xFF;
    if(!interrupt::register_irq_handler(irq, packet_handler, desc)){
        logging::logf(logging::log_level::ERROR, "rtl8139: Unable to register IRQ handler %u\n", irq);
    }

    // 7. Set IMR + ISR

    logging::logf(logging::log_level::TRACE, "rtl8139: IRQ :%u\n", uint64_t(irq));

    // Enable some interrupts
    out_word(iobase + IMR, RX_OK | TX_OK | TX_ERR);

    // 8. Set RCR (Receive Configuration Register)

    out_dword(iobase + RCR, RCR_AAP | RCR_APM | RCR_AM | RCR_AB | RCR_WRAP);

    // 9. Enable RX and TX

    out_dword(iobase+ RX_MISSED, 0x0);
    out_byte(iobase + CMD, 0x0C); // Sets the RE and TE bits high

    // 10. Get the mac address

    size_t mac = 0;

    for(size_t i = 0; i < 6; ++i){
        mac |= uint64_t(in_byte(iobase + i)) << ((5 - i) * 8);
    }

    interface.mac_address = mac;

    logging::logf(logging::log_level::TRACE, "rtl8139: MAC Address %h \n", mac);
}

void rtl8139::finalize_driver(network::interface_descriptor& interface){
    auto* desc = static_cast<rtl8139_t*>(interface.driver_data);
    desc->interface = &interface;
}
