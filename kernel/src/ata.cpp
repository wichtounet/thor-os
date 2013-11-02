#include "ata.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"

void ata_read_sectors(uint8_t controller, bool slave, std::size_t start, uint8_t count, void* destination){
    uint8_t drive = slave ? 0xF0 : 0xE0;

    out_byte(controller + 0x6, drive | ((start >> 24) & 0x0F)); //TODO Perhaps not necessary to add ored lba value

    out_byte(controller + 0x1, 0x00);
    out_byte(controller + 0x2, count);

    //Send the LBA28 value
    out_byte(controller + 0x3, static_cast<uint8_t>(start));
    out_byte(controller + 0x4, static_cast<uint8_t>(start >> 8));
    out_byte(controller + 0x5, static_cast<uint8_t>(start >> 16));
    //out_byte(controller + 0x6, drive | ((start >> 24) & 0x0F)); //TODO Perhaps not necessary to add ored lba value

    //Send the read sectors command
    out_byte(controller + 0x7, 0x20);

    uint16_t* buffer = reinterpret_cast<uint16_t*>(destination);

    for(uint8_t sector = 0; sector < count; ++sector){
        sleep_ms(1);

        while (!(in_byte(0x1F7) & 0x08)) {
            __asm__ __volatile__ ("nop; nop;");
        }

        for(int i = 0; i < 256; ++i){
            (*buffer++) = in_word(controller + 0x0);
        }
    }
}
