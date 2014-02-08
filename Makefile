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

programs/one/a.out: force_look tlib/libtlib.a
	cd programs/one; ${MAKE} a.out

programs/hello/a.out: force_look tlib/libtlib.a
	cd programs/hello; ${MAKE} a.out

programs/long/a.out: force_look tlib/libtlib.a
	cd programs/long; ${MAKE} a.out

programs/longone/a.out: force_look tlib/libtlib.a
	cd programs/longone; ${MAKE} a.out

programs/longtwo/a.out: force_look tlib/libtlib.a
	cd programs/longtwo; ${MAKE} a.out

programs/loop/a.out: force_look tlib/libtlib.a
	cd programs/loop; ${MAKE} a.out

programs/keyboard/a.out: force_look tlib/libtlib.a
	cd programs/keyboard; ${MAKE} a.out

programs/tsh/tsh: force_look tlib/libtlib.a
	cd programs/tsh; ${MAKE} tsh

hdd.img:
	dd if=/dev/zero of=hdd.img bs=516096c count=1000
	(echo n; echo p; echo 1; echo ""; echo ""; echo t; echo c; echo a; echo 1; echo w;) | sudo fdisk -u -C1000 -S63 -H16 hdd.img

thor.flp: hdd.img bootloader/stage1.bin bootloader/stage2.bin kernel/kernel.bin programs/one/a.out programs/hello/a.out programs/long/a.out programs/loop/a.out programs/longone/a.out programs/longtwo/a.out programs/keyboard/a.out programs/tsh/tsh
	mkdir -p mnt/fake/
	dd if=bootloader/stage1.bin of=hdd.img conv=notrunc
	dd if=bootloader/stage2.bin of=hdd.img seek=1 conv=notrunc
	sudo /sbin/losetup -o1048576 /dev/loop0 hdd.img
	sudo /usr/sbin/mkdosfs -F32 /dev/loop0
	sudo /bin/mount -t vfat /dev/loop0 mnt/fake/
	sudo /bin/cp kernel/kernel.bin mnt/fake/
	sudo /bin/cp programs/one/a.out mnt/fake/one
	sudo /bin/cp programs/hello/a.out mnt/fake/hello
	sudo /bin/cp programs/long/a.out mnt/fake/long
	sudo /bin/cp programs/longone/a.out mnt/fake/longone
	sudo /bin/cp programs/longtwo/a.out mnt/fake/longtwo
	sudo /bin/cp programs/loop/a.out mnt/fake/loop
	sudo /bin/cp programs/keyboard/a.out mnt/fake/keyboard
	sudo /bin/cp programs/tsh/tsh mnt/fake/tsh
	sleep 0.1
	sudo /bin/umount mnt/fake/
	sudo /sbin/losetup -d /dev/loop0

qemu: default
	qemu-kvm -cpu host -vga std -hda hdd.img

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
	cd programs/one; $(MAKE) clean
	cd programs/hello; $(MAKE) clean
	cd programs/long;$(MAKE) clean
	cd programs/loop; $(MAKE) clean
	cd programs/longone; $(MAKE) clean
	cd programs/longtwo; $(MAKE) clean
	cd programs/keyboard; $(MAKE) clean
	cd programs/tsh; $(MAKE) clean
	cd tlib/; $(MAKE) clean
	rm -f *.bin
	rm -f *.flp
