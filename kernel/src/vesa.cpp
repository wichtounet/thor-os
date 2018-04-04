//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <types.hpp>
#include <algorithms.hpp>

#include "vesa.hpp"
#include "paging.hpp"
#include "virtual_allocator.hpp"
#include "early_memory.hpp"
#include "console.hpp"
#include "kernel.hpp"

#include "fs/sysfs.hpp"

/**
 * VESA Drawing implementation.
 *
 * This implemenation only supports 32bpp. Moreover, it also erases the
 * reserved bits of each pixel.
 */

namespace {

uint32_t* screen;

size_t x_shift;
size_t y_shift;

size_t red_shift;
size_t blue_shift;
size_t green_shift;

size_t total_size;

#include <tlib/Liberation.inl>

} //end of anonymous namespace

bool vesa::enabled(){
    return early::vesa_enabled();
}

void vesa::disable(){
    early::vesa_enabled(false);
}

bool vesa::init(){
    auto& block = *reinterpret_cast<vesa::mode_info_block_t*>(early::vesa_mode_info_address);

    total_size = static_cast<size_t>(block.height) * block.bytes_per_scan_line;

    //Get the physicaladdress of the LFB
    auto physical = block.linear_video_buffer;

    auto first_page = paging::page_align(physical);
    auto left_padding = static_cast<uintptr_t>(physical) - first_page;

    auto bytes = left_padding + total_size;
    auto pages = paging::pages(bytes);

    auto virt = virtual_allocator::allocate(pages);

    if(!virt){
        return false;
    }

    if(!paging::map_pages(virt, physical, pages)){
        return false;
    }

    x_shift = 1;
    y_shift = block.bytes_per_scan_line / 4;

    red_shift = block.linear_red_mask_position;
    blue_shift = block.linear_blue_mask_position;
    green_shift = block.linear_green_mask_position;

    screen = reinterpret_cast<uint32_t*>(virt);

    sysfs::set_constant_value(sysfs::get_sys_path(), path("/vesa/enabled"), "true");
    sysfs::set_constant_value(sysfs::get_sys_path(), path("/vesa/resolution/width"), std::to_string(block.width));
    sysfs::set_constant_value(sysfs::get_sys_path(), path("/vesa/resolution/height"), std::to_string(block.height));

    return true;
}

uint64_t vesa::get_width(){
    auto& block = *reinterpret_cast<vesa::mode_info_block_t*>(early::vesa_mode_info_address);
    return block.width;
}

uint64_t vesa::get_height(){
    auto& block = *reinterpret_cast<vesa::mode_info_block_t*>(early::vesa_mode_info_address);
    return block.height;
}

uint64_t vesa::get_x_shift(){
    return x_shift;
}

uint64_t vesa::get_y_shift(){
    return y_shift;
}

uint64_t vesa::get_bytes_per_scan_line(){
    auto& block = *reinterpret_cast<vesa::mode_info_block_t*>(early::vesa_mode_info_address);
    return block.bytes_per_scan_line;
}

uint64_t vesa::get_red_shift(){
    return red_shift;
}

uint64_t vesa::get_green_shift(){
    return green_shift;
}

uint64_t vesa::get_blue_shift(){
    return blue_shift;
}

uint32_t vesa::make_color(uint8_t r, uint8_t g, uint8_t b){
    return (r << red_shift) + (g << green_shift) + (b << blue_shift);
}

void vesa::draw_hline(void* buffer, size_t x, size_t y, size_t w, uint32_t color){
    auto screen = static_cast<uint32_t*>(buffer);

    auto where = x + y * y_shift;

    for(size_t i = 0; i < w; ++i){
        screen[where + i] = color;
    }
}

void vesa::draw_hline(size_t x, size_t y, size_t w, uint32_t color){
    draw_hline(screen, x, y, w, color);
}

void vesa::draw_vline(void* buffer, size_t x, size_t y, size_t h, uint32_t color){
    auto screen = static_cast<uint32_t*>(buffer);

    auto where = x + y * y_shift;

    for(size_t i = 0; i < h; ++i){
        screen[where + i * y_shift] = color;
    }
}

void vesa::draw_vline(size_t x, size_t y, size_t h, uint32_t color){
    draw_vline(screen, x, y, h, color);
}

void vesa::draw_char(void* buffer, size_t x, size_t y, char c, uint32_t color){
    auto screen = static_cast<uint32_t*>(buffer);

    auto where = x + y * y_shift;

    auto font_char = &Liberation_VESA_data[c * 16];

    for(size_t i = 0; i < 16; ++i){
        for(size_t j = 0; j < 8; ++j){
            if(font_char[i] & (1 << (8 - j))){
                screen[where+j] = color;
            } else {
                screen[where+j] = 0;
            }
        }

        where += y_shift;
    }
}

void vesa::draw_char(size_t x, size_t y, char c, uint32_t color){
    draw_char(screen, x, y, c, color);
}

void vesa::draw_rect(void* buffer, size_t x, size_t y, size_t w, size_t h, uint32_t color){
    auto screen = static_cast<uint32_t*>(buffer);

    auto where = x + y * y_shift;

    for(size_t j = 0; j < h; ++j){
        for(size_t i = 0; i < w; ++i){
            screen[where + i] = color;
        }

        where += y_shift;
    }
}

void vesa::draw_rect(size_t x, size_t y, size_t w, size_t h, uint32_t color){
    draw_rect(screen, x, y, w, h, color);
}

void vesa::move_lines_up(void* buffer, size_t y, size_t x, size_t w, size_t lines, size_t n){
    auto screen = static_cast<uint32_t*>(buffer);

    for(size_t i = 0; i < lines; ++i){
        auto destination = reinterpret_cast<size_t*>(screen + (y - n + i) * y_shift + x);
        auto source = reinterpret_cast<size_t*>(screen + (y + i) * y_shift + x);

        std::copy_n(source, w / 2, destination);
    }
}

void vesa::move_lines_up(size_t y, size_t x, size_t w, size_t lines, size_t n){
    move_lines_up(screen, y, x, w, lines, n);
}

void* vesa::create_buffer(){
    return new char[total_size];
}

void vesa::redraw(const char* buffer){
    std::copy_n(buffer, total_size, reinterpret_cast<char*>(screen));
}

void vesa::save(char* buffer){
    std::copy_n(reinterpret_cast<const char*>(screen), total_size, buffer);
}
