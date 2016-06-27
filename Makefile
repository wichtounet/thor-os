.PHONY: default clean force_look qemu bochs debug

default: thor.flp

kernel/kernel.bin: force_look
	cd kernel; $(MAKE)

tlib/libtlib.a: force_look
	cd tlib; $(MAKE)

bootloader/stage1.bin: force_look
	cd bootloader; $(MAKE) stage1.bin

bootloader/stage2.bin: force_look
	cd bootloader; $(MAKE) stage2.bin

programs: force_look tlib/libtlib.a
	cd programs/; ${MAKE} dist

compile: bootloader/stage1.bin bootloader/stage2.bin kernel/kernel.bin programs

hdd.img:
	dd if=/dev/zero of=hdd.img bs=516096c count=1000
	(echo n; echo p; echo 1; echo ""; echo ""; echo t; echo c; echo a; echo 1; echo w;) | sudo fdisk -u -C1000 -S63 -H16 hdd.img

thor.flp: hdd.img bootloader/stage1.bin bootloader/stage2.bin kernel/kernel.bin programs
	mkdir -p mnt/fake/
	dd if=bootloader/stage1.bin of=hdd.img conv=notrunc
	dd if=bootloader/stage2.bin of=hdd.img seek=1 conv=notrunc
	sudo /sbin/losetup -o1048576 /dev/loop0 hdd.img
	sudo /usr/sbin/mkdosfs -F32 /dev/loop0
	sudo /bin/mount -t vfat /dev/loop0 mnt/fake/
	sudo mkdir mnt/fake/bin/
	sudo mkdir mnt/fake/sys/
	sudo mkdir mnt/fake/dev/
	sudo /bin/cp kernel/kernel.bin mnt/fake/
	sudo /bin/cp programs/dist/* mnt/fake/bin/
	sleep 0.1
	sudo /bin/umount mnt/fake/
	sudo /sbin/losetup -d /dev/loop0

qemu: default
	qemu-system-x86_64 -enable-kvm -cpu host -vga std -hda hdd.img

bochs: default
	echo "c" > commands
	bochs -qf tools/bochsrc.txt -rc commands
	rm commands

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
	cd kernel; $(MAKE) clean
	cd programs/; $(MAKE) clean
	cd tlib/; $(MAKE) clean
	rm -f *.bin
	rm -f *.flp
