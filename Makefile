.PHONY: all install doc clean

all: build/gravcalc.pbw

build/gravcalc.pbw: src/gravcalc.c src/config.h src/fixed.h src/utility.h
	pebble build

install: all
	pebble install

doc:
	doxygen Doxyfile

test:
	make -C tests

runtest: test
	./tests/unittests

clean:
	rm -rf build
