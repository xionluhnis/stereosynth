CC := g++ -std=c++11
INCL := -I./include/ -I./src -I/usr/include/
RESULT := echo "[Passed] target" || echo "[Failed] target"
TEST := -g -DDEBUG_STRICT_TEST=1 -o bin/test_target tests/target.cpp && bin/test_target && $(RESULT)

OPTI_FLAGS := -O6 -w -s -ffast-math -fomit-frame-pointer -fstrength-reduce -msse2 -funroll-loops -fPIC
BASE_FLAGS := -std=c++11 -DNDEBUG -DUNIX_MODE -DMEXMODE -fPIC -ftls-model=global-dynamic
LIBS_FLAGS := -Wl,--export-dynamic -Wl,-e,mexFunction -shared
MEX := mex -v CXXOPTIMFLAGS='$$CXXOPTIMFLAGS $(OPTI_FLAGS)' CXXFLAGS='$$CXXFLAGS $(BASE_FLAGS)' CXXLIBS='$$CXXLIBS ${LIBS_FLAGS}' ${INCL}

mex:
	mkdir bin 2>/dev/null
	$(MEX) src/int_single_nnf.cpp -output bin/isnnf -output bin/isnnf
	$(MEX) src/int_k_nnf.cpp -output bin/iknnf -output bin/iknnf

old_mex:
	bash build.sh

clean:
	rm -rf bin

all: mex
.phony: mex

test:
	mkdir bin 2>/dev/null
	$(CC) $(INCL) $(subst target,scanline,$(TEST))
	$(CC) $(INCL) $(subst target,rng_uniform,$(TEST))
	$(CC) $(INCL) $(subst target,int_single_nnf,$(TEST))
	$(CC) $(INCL) $(subst target,int_k_nnf,$(TEST))
	
