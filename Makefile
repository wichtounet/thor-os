.PHONY: default clean bootloader kernel micro_kernel force_look qemu bochs debug

default: bootloader micro_kernel kernel thor.flp

bootloader: force_look
	cd bootloader; $(MAKE)

micro_kernel: force_look
	cd micro_kernel; $(MAKE)

kernel: force_look
	cd kernel; $(MAKE)

filler.bin: kernel/kernel.bin micro_kernel/micro_kernel.bin bootloader/stage1.bin bootloader/stage2.bin bootloader/padding.bin
	bash fill.bash

thor.flp: filler.bin
	cat bootloader/stage1.bin > thor.bin
	cat bootloader/stage2.bin >> thor.bin
	cat bootloader/padding.bin >> thor.bin
	cat micro_kernel/micro_kernel.bin >> thor.bin
	cat kernel/kernel.bin >> thor.bin
	cat filler.bin >> thor.bin
	dd status=noxfer conv=notrunc if=thor.bin of=thor.flp

qemu: thor.flp
	qemu-kvm -cpu host -fda thor.flp -hda hdd.img -boot order=a

bochs: thor.flp
	echo "c" > commands
	bochs -qf bochsrc.txt -rc commands
	rm commands

debug: thor.flp
	echo "c" > commands
	bochs -qf debug_bochsrc.txt -rc commands
	rm commands

force_look:
	true

clean:
	cd bootloader; $(MAKE) clean
	cd kernel; $(MAKE) clean
	cd micro_kernel; $(MAKE) clean
	rm -f *.bin
	rm -f *.flp