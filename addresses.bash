#!/bin/bash

function generate_address {
    address=`readelf --symbols micro_kernel.g | grep $1 | xargs | cut -d ' ' -f 2`
    hex_address="0x$address"
    hex_offset=`echo "obase=16; $(($hex_address+0x1000))" | bc`

    echo "#define asm_$1 0x$hex_offset" >> src/addresses.hpp
}

echo "#ifndef ADDRESSES_H" > src/addresses.hpp
echo "#define ADDRESSES_H" >> src/addresses.hpp

generate_address "register_irq_handler"

echo "#endif" >> src/addresses.hpp
