.PHONY: all install doc clean

all: build/gravcalc.pbw

build/gravcalc.pbw: src/gravcalc.c
	pebble build

install: all
	pebble install

doc:
	doxygen Doxyfile

clean:
	rm -rf build
