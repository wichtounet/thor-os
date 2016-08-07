.PHONY: dist default clean force_look

include ../cpp.mk

PROGRAMS=long loop longone longtwo keyboard tsh cpuid shutdown reboot args stat cat which readelf touch mkdir rm date ls lspci lse820 mount df uptime writer divzero odin

default: dist

tlib/libtlib.a: force_look
	cd ../tlib; $(MAKE)

force_look:
	true

dist: tlib/libtlib.a
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Build all programs"
	@ $(foreach var,$(PROGRAMS),cd $(var); $(MAKE); cd ..;)
	@ mkdir -p dist
	@ $(foreach var,$(PROGRAMS),cp $(var)/debug/$(var) dist/;)
	@ strip dist/*

clean:
	@ echo -e "Clean all programs"
	@ $(foreach var,$(PROGRAMS),cd $(var); $(MAKE) clean; cd ..;)
	@ rm -rf dist
