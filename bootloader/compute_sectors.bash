#!/bin/bash

size_1=`stat -c%s ../kernel/kernel.bin`
size_2=`stat -c%s ../micro_kernel/micro_kernel.bin`
size_3=`stat -c%s ../filler.bin`

total_size=$(( $size_1 + $size_2 + $size_3 ))
sectors=$(( $total_size / 512 ))

echo "sectors equ $sectors" > sectors.asm
