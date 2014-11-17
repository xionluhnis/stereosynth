INCL := -I./include/ -I./src -I/usr/include/
RESULT := echo "[Passed] target" || echo "[Failed] target"
TEST := -DDEBUG_STRICT_TEST=1 -o bin/test_target tests/target.cpp && bin/test_target && $(RESULT)

mex:
	bash build.sh

clean:
	rm -rf bin

all: mex
.phony: mex

test:
	mkdir bin 2>/dev/null
	g++ -std=c++11 $(INCL) $(subst target,scanline,$(TEST))
