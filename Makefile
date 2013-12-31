.PHONY: default clean force_look qemu bochs debug sectors

default: thor.flp

kernel/kernel.bin: force_look
	cd kernel; $(MAKE)

filler.bin: kernel/kernel.bin
	bash fill.bash

sectors: force_look filler.bin
	cd bootloader; $(MAKE) sectors

bootloader/bootloader.bin: force_look sectors
	cd bootloader; $(MAKE)

thor.flp: bootloader/bootloader.bin
	dd if=bootloader/stage1.bin of=hdd.img conv=notrunc
	dd if=bootloader/stage2.bin of=hdd.img seek=1 conv=notrunc
	sudo /sbin/losetup -o1048576 /dev/loop0 hdd.img
	sudo /bin/mount -t vfat /dev/loop0 /mnt/fake_cdrom/
	sudo /bin/cp kernel/kernel.bin /mnt/fake_cdrom/
	sudo /bin/umount /mnt/fake_cdrom/
	sudo /sbin/losetup -d /dev/loop0

qemu: default
	qemu-kvm -cpu host -hda hdd.img

bochs: default
	echo "c" > commands
	bochs -qf bochsrc.txt -rc commands
	rm commands

debug: default
	echo "c" > commands
	bochs -qf debug_bochsrc.txt -rc commands
	rm commands

force_look:
	true

clean:
	cd bootloader; $(MAKE) clean
	cd kernel; $(MAKE) clean
	rm -f *.bin
	rm -f *.flp