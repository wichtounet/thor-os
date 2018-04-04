//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <algorithms.hpp>

#include "drivers/mouse.hpp"

#include "interrupts.hpp"
#include "kernel_utils.hpp"
#include "logging.hpp"
#include "vesa.hpp"
#include "stdio.hpp"

namespace {

#ifdef THOR_CONFIG_MOUSE_VERBOSE
#define verbose_logf(...) logging::logf(__VA_ARGS__)
#else
#define verbose_logf(...)
#endif

constexpr const uint16_t DATA_PORT = 0x60;
constexpr const uint16_t STATUS_PORT = 0x64;

uint8_t cycle = 0; // The interrupt cycles through different information

uint8_t previous_flags = 0;
uint8_t mouse_packet[3];
uint16_t position_x = 0;
uint16_t position_y = 0;

void mouse_handler(interrupt::syscall_regs*, void*){
    mouse_packet[cycle++] = in_byte(DATA_PORT);

    if(cycle == 1){ // was 0
        int8_t flags = mouse_packet[0];

        // If bit 3 is not set, drop the packet
        if((flags & (1 << 3)) == 0){
            cycle = 0;
            return;
        }
    }

    if(cycle == 3){
        cycle = 0;

        int8_t flags = mouse_packet[0];

        // Discard overflow packets
        if(flags & ((1 << 6) | (1 << 7))){
            return;
        }

        // Compute the 9bit delta values
        int16_t delta_x = mouse_packet[1] - ((flags << 4) & 0x100);
        int16_t delta_y = mouse_packet[2] - ((flags << 3) & 0x100);

        // Reverse the y direction
        delta_y = -delta_y;

        if(delta_y || delta_x){
            position_x = std::max(int16_t(position_x) + delta_x, 0);
            position_y = std::max(int16_t(position_y) + delta_y, 0);

            if(vesa::enabled()){
                position_x = std::min(position_x, uint16_t(vesa::get_width()));
                position_y = std::min(position_y, uint16_t(vesa::get_height()));
            }

            verbose_logf(logging::log_level::TRACE, "mouse: moved %d:%d \n", int64_t(position_x), int64_t(position_y));
        }

        if((flags & (1 << 0)) && !(previous_flags & (1 << 0))){
            verbose_logf(logging::log_level::TRACE, "mouse: left button pressed %d:%d \n", int64_t(position_x), int64_t(position_y));

            stdio::get_active_terminal().send_mouse_input(std::keycode::MOUSE_LEFT_PRESS);
        }

        if(!(flags & (1 << 0)) && (previous_flags & (1 << 0))){
            verbose_logf(logging::log_level::TRACE, "mouse: left button released %d:%d \n", int64_t(position_x), int64_t(position_y));

            stdio::get_active_terminal().send_mouse_input(std::keycode::MOUSE_LEFT_RELEASE);
        }

        if((flags & (1 << 1)) && !(previous_flags & (1 << 1))){
            verbose_logf(logging::log_level::TRACE, "mouse: right button pressed %d:%d \n", int64_t(position_x), int64_t(position_y));

            stdio::get_active_terminal().send_mouse_input(std::keycode::MOUSE_RIGHT_PRESS);
        }

        if(!(flags & (1 << 1)) && (previous_flags & (1 << 1))){
            verbose_logf(logging::log_level::TRACE, "mouse: right button released %d:%d \n", int64_t(position_x), int64_t(position_y));

            stdio::get_active_terminal().send_mouse_input(std::keycode::MOUSE_RIGHT_RELEASE);
        }

        previous_flags = flags;
    }
}

void ps2_data_wait(){
    auto timeout = 100000;
    while(timeout-- && (in_byte(STATUS_PORT) & 1) != 1){}
}

void ps2_signal_wait(){
    auto timeout = 100000;
    while(timeout-- && (in_byte(STATUS_PORT) & 2) != 0){}
}

void ps2_mouse_write(uint8_t value){
    // Send a command
    ps2_signal_wait();
    out_byte(STATUS_PORT, 0xD4);

    // Send the data
    ps2_signal_wait();
    out_byte(DATA_PORT, value);
}

uint8_t ps2_mouse_read(){
    ps2_data_wait();
    return in_byte(DATA_PORT);
}

} //end of anonymous namespace

void mouse::install(){
    if(!interrupt::register_irq_handler(12, mouse_handler, nullptr)){
        logging::logf(logging::log_level::ERROR, "mouse: Unable to register IRQ handler 12\n");
        return;
    }

    // Enable the mouse auxiliary device
    ps2_signal_wait();
    out_byte(STATUS_PORT, 0xA8);

    // Enable interrupts
    ps2_signal_wait();
    out_byte(STATUS_PORT, 0x20);
    auto status = ps2_mouse_read() | 2;
    ps2_signal_wait();
    out_byte(STATUS_PORT, 0x60);
    ps2_signal_wait();
    out_byte(DATA_PORT, status);

    // Use default settings
    ps2_mouse_write(0xF6);
    ps2_mouse_read();

    // Enable the mouse
    ps2_mouse_write(0xF4);
    ps2_mouse_read();

    logging::logf(logging::log_level::TRACE, "mouse: PS/2 mouse driver installed\n");
}

uint64_t mouse::x(){
    return position_x;
}

uint64_t mouse::y(){
    return position_y;
}
