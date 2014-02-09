.PHONY: default clean force_look

default: dist

tlib/libtlib.a: force_look
	cd ../tlib; $(MAKE)

one: force_look tlib/libtlib.a
	cd one; $(MAKE) a.out

hello: force_look tlib/libtlib.a
	cd hello; $(MAKE) a.out

long: force_look tlib/libtlib.a
	cd long; $(MAKE) a.out

longone: force_look tlib/libtlib.a
	cd longone; $(MAKE) a.out

longtwo: force_look tlib/libtlib.a
	cd longtwo; $(MAKE) a.out

loop: force_look tlib/libtlib.a
	cd loop; $(MAKE) a.out

keyboard: force_look tlib/libtlib.a
	cd keyboard; $(MAKE) a.out

cpuid: force_look tlib/libtlib.a
	cd cpuid; $(MAKE) cpuid

tsh: force_look tlib/libtlib.a
	cd tsh; $(MAKE) tsh

dist: one hello long loop longone longtwo keyboard tsh cpuid
	mkdir -p dist
	cp one/a.out dist/one
	cp hello/a.out dist/hello
	cp long/a.out dist/long
	cp longone/a.out dist/longone
	cp longtwo/a.out dist/longtwo
	cp loop/a.out dist/loop
	cp keyboard/a.out dist/keyboard
	cp cpuid/cpuid dist/
	cp tsh/tsh dist/

force_look:
	true

clean:
	cd one; $(MAKE) clean
	cd hello; $(MAKE) clean
	cd long;$(MAKE) clean
	cd loop; $(MAKE) clean
	cd longone; $(MAKE) clean
	cd longtwo; $(MAKE) clean
	cd keyboard; $(MAKE) clean
	cd tsh; $(MAKE) clean
	cd cpuid; $(MAKE) clean
	rm -rf dist
