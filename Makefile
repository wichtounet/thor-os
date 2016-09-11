SHELL=/bin/bash

.PHONY: default clean force_look qemu bochs debug

default: thor.flp

/tmp/ReOS/hdd.img:
	mkdir -p /tmp/ReOS
	dd if=/dev/zero of=/tmp/ReOS/hdd.img bs=516096c count=1000
	(echo n; echo p; echo 1; echo ""; echo ""; echo t; echo c; echo a; echo w;) | fdisk -u -C1000 -S63 -H16 /tmp/ReOS/hdd.img

/tmp/ReOS/bootloader/stage1.bin:
	cd bootloader; $(MAKE) /tmp/ReOS/bootloader/stage1.bin

/tmp/ReOS/bootloader/stage2.bin:
	cd bootloader; $(MAKE) /tmp/ReOS/bootloader/stage2.bin

/tmp/ReOS/init/debug/init.bin: force_look
	cd init; $(MAKE)

/tmp/ReOS/kernel/debug/kernel.bin: force_look
	cd kernel; $(MAKE)

/tmp/ReOS/tlib/debug/libtlib.a: force_look
	cd tlib; $(MAKE)

/tmp/ReOS/programs: force_look /tmp/ReOS/tlib/debug/libtlib.a
	cd programs/; ${MAKE} dist

compile: /tmp/ReOS/bootloader/stage1.bin /tmp/ReOS/bootloader/stage2.bin /tmp/ReOS/init/debug/init.bin /tmp/ReOS/kernel/debug/kernel.bin /tmp/ReOS/programs

thor.flp: /tmp/ReOS/hdd.img /tmp/ReOS/bootloader/stage1.bin /tmp/ReOS/bootloader/stage2.bin /tmp/ReOS/init/debug/init.bin /tmp/ReOS/kernel/debug/kernel.bin /tmp/ReOS/programs
	mkdir -p /tmp/ReOS/mnt/fake/
	dd if=/tmp/ReOS/bootloader/stage1.bin of=/tmp/ReOS/hdd.img conv=notrunc
	dd if=/tmp/ReOS/bootloader/stage2.bin of=/tmp/ReOS/hdd.img seek=1 conv=notrunc
	sudo /sbin/losetup -o1048576 /dev/loop0 /tmp/ReOS/hdd.img
	sudo mkdosfs -v -F32 /dev/loop0
	sudo /bin/mount -t vfat /dev/loop0 /tmp/ReOS/mnt/fake/
	sudo mkdir /tmp/ReOS/mnt/fake/bin/
	sudo mkdir /tmp/ReOS/mnt/fake/sys/
	sudo mkdir /tmp/ReOS/mnt/fake/dev/
	sudo mkdir /tmp/ReOS/mnt/fake/proc/
	sudo /bin/cp /tmp/ReOS/init/debug/init.bin /tmp/ReOS/mnt/fake/
	sudo /bin/cp /tmp/ReOS/kernel/debug/kernel.bin /tmp/ReOS/mnt/fake/
	sudo /bin/cp /tmp/ReOS/programs/dist/* /tmp/ReOS/mnt/fake/bin/
	sleep 1
	sudo /bin/umount /tmp/ReOS/mnt/fake/
	sudo /sbin/losetup -d /dev/loop0

qemu: default
	touch virtual.log
	sudo qemu-system-x86_64 -enable-kvm -cpu host -serial file:virtual.log -netdev tap,helper=/usr/libexec/qemu-bridge-helper,id=thor_net0 -device rtl8139,netdev=thor_net0,id=thor_nic0 -vga std -hda /tmp/ReOS/hdd.img &
	echo "Reading kernel log (Ctrl+C for exit)"
	tail -f virtual.log
	kill %1

bochs: default
	echo "c" > commands
	bochs -qf tools/bochsrc.txt -rc commands
	rm commands

bochs_simple: default
	bochs -qf tools/bochsrc.txt

debug: default
	echo "c" > commands
	bochs -qf tools/debug_bochsrc.txt -rc commands
	rm commands

gdb: default
	bochs -qf tools/gdb_bochsrc.txt

force_look:
	true

clean:
	cd bootloader; $(MAKE) clean
	cd init; $(MAKE) clean
	cd kernel; $(MAKE) clean
	cd programs/; $(MAKE) clean
	cd tlib/; $(MAKE) clean
	rm -f *.bin
	rm -f *.flp
	rm -rf /tmp/ReOS
