#!/bin/bash

size=`stat -c%s kernel.bin`
let filler_size=4096-$size
dd if=/dev/zero of=filler.bin bs=1 count=$filler_size
