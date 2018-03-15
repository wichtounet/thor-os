.PHONY: default clean force_look qemu bochs debug mount_fat check_fat umount_fat

default: thor.flp

kernel/debug/kernel.bin: force_look
	cd kernel; $(MAKE)

init/debug/init.bin: force_look
	cd init; $(MAKE)

tlib/debug/libtlib.a: force_look
	cd tlib; $(MAKE)

bootloader/stage1.bin: force_look
	cd bootloader; $(MAKE) stage1.bin

bootloader/stage2.bin: force_look
	cd bootloader; $(MAKE) stage2.bin

programs: force_look tlib/debug/libtlib.a
	cd programs/; ${MAKE} dist

compile: bootloader/stage1.bin bootloader/stage2.bin init/debug/init.bin kernel/debug/kernel.bin programs

hdd.img:
	dd if=/dev/zero of=hdd.img bs=516096c count=1000
	(echo n; echo p; echo 1; echo ""; echo ""; echo t; echo c; echo a; echo 1; echo w;) | sudo fdisk -u -C1000 -S63 -H16 hdd.img

thor.flp: hdd.img bootloader/stage1.bin bootloader/stage2.bin init/debug/init.bin kernel/debug/kernel.bin programs
	mkdir -p mnt/fake/
	dd if=bootloader/stage1.bin of=hdd.img conv=notrunc
	dd if=bootloader/stage2.bin of=hdd.img seek=1 conv=notrunc
	sudo /sbin/losetup -o1048576 /dev/loop0 hdd.img
	sudo mkdosfs -v -F32 /dev/loop0
	sudo /bin/mount -t vfat /dev/loop0 mnt/fake/
	sudo mkdir mnt/fake/bin/
	sudo mkdir mnt/fake/sys/
	sudo mkdir mnt/fake/dev/
	sudo mkdir mnt/fake/proc/
	sudo /bin/cp init/debug/init.bin mnt/fake/
	sudo /bin/cp kernel/debug/kernel.bin mnt/fake/
	sudo /bin/cp programs/dist/* mnt/fake/bin/
	sleep 0.1
	sudo /bin/umount mnt/fake/
	sudo /sbin/losetup -d /dev/loop0

qemu: default
	touch virtual.log
	sudo qemu-system-x86_64 -enable-kvm -cpu host -serial file:virtual.log -netdev tap,helper=/usr/libexec/qemu-bridge-helper,id=thor_net0 -device rtl8139,netdev=thor_net0,id=thor_nic0 -vga std -hda hdd.img &
	echo "Reading kernel log (Ctrl+C for exit)"
	tail -f virtual.log
	kill %1

qemu_run: default
	touch virtual.log
	sudo run-qemu -enable-kvm -cpu host -serial file:virtual.log -vga std -hda hdd.img &
	echo "Reading kernel log (Ctrl+C for exit)"
	tail -f virtual.log
	kill %1

qemu_user: default
	touch virtual.log
	sudo qemu-system-x86_64 -enable-kvm -cpu host -serial file:virtual.log -net user,vlan=0 -net nic,model=rtl8139,vlan=0,macaddr=52:54:00:12:34:56 -net socket,listen=localhost:1234 -net dump,vlan=0,file=thor.pcap -vga std -hda hdd.img &
	echo "Reading kernel log (Ctrl+C for exit)"
	tail -f virtual.log
	kill %1

qemu_user_slave: default
	touch slave.log
	sudo qemu-system-x86_64 -enable-kvm -cpu host -serial file:slave.log -net nic,model=rtl8139,vlan=0,macaddr=52:54:00:12:34:57 -net socket,connect=localhost:1234,vlan=0 -vga std -hda hdd.img &
	echo "Reading kernel log (Ctrl+C for exit)"
	tail -f slave.log
	kill %1

bochs: default
	echo "c" > commands
	bochs -qf tools/bochsrc.txt -rc commands
	rm commands

bochs_simple: default
	bochs -qf tools/bochsrc.txt

mount_fat:
	sudo /sbin/losetup -o1048576 /dev/loop0 hdd.img
	sudo /bin/mount -t vfat /dev/loop0 mnt/fake/

check_fat:
	sudo /sbin/losetup -o1048576 /dev/loop0 hdd.img
	sudo /bin/mount -t vfat /dev/loop0 mnt/fake/
	sudo fsck.fat -v -n /dev/loop0 || true
	sudo /bin/umount mnt/fake/
	sudo /sbin/losetup -d /dev/loop0

umount_fat:
	sudo /bin/umount mnt/fake/
	sudo /sbin/losetup -d /dev/loop0

umount_loop:
	sudo /sbin/losetup -d /dev/loop0

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
