.PHONY: dist default clean force_look

PROGRAMS=long loop longone longtwo keyboard tsh cpuid shutdown reboot args stat cat which readelf touch mkdir rm date ls mount df uptime writer

default: dist

tlib/libtlib.a: force_look
	cd ../tlib; $(MAKE)

force_look:
	true

dist: tlib/libtlib.a
	$(foreach var,$(PROGRAMS),cd $(var); $(MAKE); cd ..;)
	mkdir -p dist
	$(foreach var,$(PROGRAMS),cp $(var)/$(var) dist/;)
	strip dist/*

clean:
	echo $(PROGRAMS_MAKE)
	echo $(PROGRAMS_CLEAN)
	echo $(PROGRAMS)
	$(foreach var,$(PROGRAMS),cd $(var); $(MAKE) clean; cd ..;)
	rm -rf dist
