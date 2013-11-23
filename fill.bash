#!/bin/bash

size=`stat -c%s kernel/kernel.bin`
let filler_size=20480-$size
dd if=/dev/zero of=filler.bin bs=1 count=$filler_size
