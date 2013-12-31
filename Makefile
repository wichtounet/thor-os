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
	dd if=bootloader/stage1.bin of=hdd.img bs=446 count=1 conv=notrunc
	dd if=bootloader/stage2.bin of=hdd.img bs=512 count=1 seek=1 conv=notrunc
	cat bootloader/bootloader.bin > thor.bin
	cat kernel/kernel.bin >> thor.bin
	cat filler.bin >> thor.bin
	dd status=noxfer conv=notrunc if=thor.bin of=thor.flp

qemu: default
	qemu-kvm -cpu host -fda thor.flp -hda hdd.img -boot order=a

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