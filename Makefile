
mex:
	bash build.sh

clean:
	rm -rf bin

all: mex
.phony: mex

test:
	mkdir bin 2>/dev/null
	g++ -std=c++11 -I./include/ -I./src -I/usr/include/ -o bin/test_scanline tests/scanline.cpp && bin/test_scanline