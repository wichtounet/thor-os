.PHONY: dist default clean force_look

include ../cpp.mk

PROGRAMS_DIRS=$(filter-out dist/, $(filter %/, $(wildcard */)))
PROGRAMS=$(PROGRAMS_DIRS:%/=%)

default: dist

/tmp/ReOS/tlib/debug/libtlib.a: force_look
	cd ../tlib; $(MAKE)

force_look:
	true

dist: /tmp/ReOS/tlib/debug/libtlib.a
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Build all programs"
	@ $(foreach var,$(PROGRAMS),cd $(var); $(MAKE); cd ..;)
	@ mkdir -p /tmp/ReOS/programs/dist
	@ $(foreach var,$(PROGRAMS),cp /tmp/ReOS/programs/$(var)/debug/$(var) /tmp/ReOS/programs/dist/;)
	@ strip /tmp/ReOS/programs/dist/*

clean:
	@ echo -e "Clean all programs"
	@ $(foreach var,$(PROGRAMS),cd $(var); $(MAKE) clean; cd ..;)
	@ rm -rf /tmp/ReOS/programs/dist