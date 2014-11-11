
mex:
	bash build.sh

clean:
	rm -rf bin/*

all: mex
.phony: mex
