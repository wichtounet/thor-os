default: thor.flp

bootloader.bin: force_look
	cd bootloader; $(MAKE)

micro_kernel.bin: force_look
	cd micro_kernel; $(MAKE)

kernel.bin: force_look
	cd kernel; $(MAKE)

filler.bin: kernel.bin
	bash fill.bash

thor.flp: bootloader.bin micro_kernel.bin kernel.bin filler.bin
	cat bootloader/bootloader.bin > thor.bin
	cat micro_kernel/micro_kernel.bin >> thor.bin
	cat kernel/kernel.bin >> thor.bin
	cat filler.bin >> thor.bin
	dd status=noxfer conv=notrunc if=thor.bin of=thor.flp

qemu: thor.flp
	qemu-kvm -cpu host -fda thor.flp -hda hdd.img

bochs: thor.flp
	echo "c" > commands
	bochs -qf bochsrc.txt -rc commands
	rm commands

debug: thor.flp
	echo "c" > commands
	bochs -qf bochsrc.txt -rc commands
	rm commands

force_look:
	true

clean:
	cd bootloader; $(MAKE) clean
	cd kernel; $(MAKE) clean
	cd micro_kernel; $(MAKE) clean
	rm -f *.bin
	rm -f *.flp