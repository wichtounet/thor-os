#!/bin/bash

size=`stat -c%s kernel/kernel.bin`
better_size=$(( ($size / 512 + 1) * 512 ))
let filler_size=$better_size-$size
dd if=/dev/zero of=filler.bin bs=1 count=$filler_size
