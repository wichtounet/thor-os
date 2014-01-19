.PHONY: default clean force_look qemu bochs debug

default: thor.flp

kernel/kernel.bin: force_look
	cd kernel; $(MAKE)

bootloader/stage1.bin: force_look
	cd bootloader; $(MAKE) stage1.bin

bootloader/stage2.bin: force_look
	cd bootloader; $(MAKE) stage2.bin

programs/one/a.out: force_look
	cd programs/one; ${MAKE} a.out

programs/hello/a.out: force_look
	cd programs/hello; ${MAKE} a.out

programs/long/a.out: force_look
	cd programs/long; ${MAKE} a.out

programs/loop/a.out: force_look
	cd programs/loop; ${MAKE} a.out

thor.flp: bootloader/stage1.bin bootloader/stage2.bin kernel/kernel.bin programs/one/a.out programs/hello/a.out programs/long/a.out programs/loop/a.out
	dd if=bootloader/stage1.bin of=hdd.img conv=notrunc
	dd if=bootloader/stage2.bin of=hdd.img seek=1 conv=notrunc
	sudo /sbin/losetup -o1048576 /dev/loop0 hdd.img
	sudo /usr/sbin/mkdosfs -F32 /dev/loop0
	sudo /bin/mount -t vfat /dev/loop0 /mnt/fake_cdrom/
	sudo /bin/cp kernel/kernel.bin /mnt/fake_cdrom/
	sudo /bin/cp programs/one/a.out /mnt/fake_cdrom/one
	sudo /bin/cp programs/hello/a.out /mnt/fake_cdrom/hello
	sudo /bin/cp programs/long/a.out /mnt/fake_cdrom/long
	sudo /bin/cp programs/loop/a.out /mnt/fake_cdrom/loop
	sleep 0.1
	sudo /bin/umount /mnt/fake_cdrom/
	sudo /sbin/losetup -d /dev/loop0

qemu: default
	qemu-kvm -cpu host -vga std -hda hdd.img

bochs: default
	echo "c" > commands
	bochs -qf bochsrc.txt -rc commands
	rm commands

debug: default
	echo "c" > commands
	bochs -qf debug_bochsrc.txt -rc commands
	rm commands

gdb: default
	bochs -qf gdb_bochsrc.txt

force_look:
	true

clean:
	cd bootloader; $(MAKE) clean
	cd kernel; $(MAKE) clean
	cd programs/one; $(MAKE) clean
	cd programs/hello; $(MAKE) clean
	rm -f *.bin
	rm -f *.flp
