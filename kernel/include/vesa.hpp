//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VESA_H
#define VESA_H

namespace vesa {

struct vbe_info_block {
    char[4] signature;             //"VESA"
    uint16_t version;              // == 0x0300 for VBE 3.0
    uint16_t[2] oem_name_ptr;
    uint8_t[4] capabilities;
    uint16_t[2] video_modes_ptr;
    uint16_t total_memory;         // as # of 64KB blocks

    //VBE 2.0
    uint16_t oem_software_version;
    uint16_t[2] vendor_name_ptr;
    uint16_t[2] product_name_ptr;
    uint16_t[2] product_revision_ptr;
    uint16_t vbe_af_version;
    uint16_t[2] accelerator_video_modes_ptr;

    uint8_t[216] reserved;
    uint8_t[256] oem_scratchpad;
} __attribute__((packed));

static_assert(sizeof(vbe_info_block) == 512, "The size of a VBE info block is 512 bytes");

struct mode_info_block {
    uint16_t mode_attributes;
    uint8_t window_a_attributes;
    uint8_t window_b_attributes;
    uint16_t window_granularity;
    uint16_t window_size;
    uint16_t segment_window_a;
    uint16_t segment_window_b;
    uint16_t[2] window_position_function;
    uint16_t pitch; //Bytes per scan line

    //OEM modes

    uint16_t width;
    uint16_t height;
    uint8_t width_char;
    uint8_t height_char;
    uint8_t memory_planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved;

    //VBE 1.2+

    uint8_t red_mask_size;
    uint8_t red_mask_position;
    uint8_t green_mask_size;
    uint8_t green_mask_position;
    uint8_t blue_mask_size;
    uint8_t blue_mask_position;
    uint8_t reserved_mask_size;
    uint8_t reserved__mask_position;
    uint8_t directcolor_attributes;

    //VBE 2.0+

    uint16_t[2] linear_video_buffer;         //LFB (Linear Framebuffer) address
    uint16_t[2] offscreen_memory;
    uint16_t offscreen_memory_size;

    uint16_t bytes_per_scan_line;
    uint8_t number_of_images_banked;
    uint8_t number_of_images_linear;
    uint8_t linear_red_mask_size;
    uint8_t linear_red_mask_position;
    uint8_t linear_green_mask_size;
    uint8_t linear_green_mask_position;
    uint8_t linear_blue_mask_size;
    uint8_t linear_blue_mask_position;
    uint8_t linear_reserved_mask_size;
    uint8_t linear_reserved_mask_position;
    uint16_t[2] maximum_pixel_clock;
    uint8_t[190] reserved;
} __attribute__((packed));

static_assert(sizeof(mode_info_block) == 256, "The size of a mode info block is 256 bytes");

} //end of vesa namespace

#endif
