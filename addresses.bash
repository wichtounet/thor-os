#!/bin/bash

function generate_address {
    address=`readelf --symbols micro_kernel.g | grep $1 | xargs | cut -d ' ' -f 2`
    hex_address="0x$address"
    hex_offset=`echo "obase=16; $(($hex_address+0x1000))" | bc`

    echo "#define asm_$1 0x$hex_offset" >> kernel/include/addresses.hpp
}

echo "#ifndef ADDRESSES_H" > kernel/include/addresses.hpp
echo "#define ADDRESSES_H" >> kernel/include/addresses.hpp

generate_address "register_irq_handler"

echo "#endif" >> kernel/include/addresses.hpp
