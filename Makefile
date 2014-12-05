CC := g++ -std=c++11
INCL := -I./include/ -I./src -I/usr/include/
RESULT := echo "[Passed] target" || echo "[Failed] target"
PNG_INCL := $(shell pkg-config --cflags libpng)
PNG_LIBS := $(shell pkg-config --libs libpng)
TEST := -g -DDEBUG_STRICT_TEST=1 -o bin/test_target tests/target.cpp && bin/test_target && $(RESULT)
TEST_WITH_PNG := -g -DDEBUG_STRICT_TEST=1 $(PNG_INCL) -o bin/test_target tests/target.cpp $(PNG_LIBS) && bin/test_target && $(RESULT)

OPTI_FLAGS := -O6 -w -s -ffast-math -fomit-frame-pointer -fstrength-reduce -msse2 -funroll-loops -fPIC
BASE_FLAGS := -std=c++11 -DNDEBUG -DUNIX_MODE -DMEXMODE -fPIC -ftls-model=global-dynamic
LIBS_FLAGS := -Wl,--export-dynamic -Wl,-e,mexFunction -shared
MEX := mex -v CXXOPTIMFLAGS='$$CXXOPTIMFLAGS $(OPTI_FLAGS)' CXXFLAGS='$$CXXFLAGS $(BASE_FLAGS)' CXXLIBS='$$CXXLIBS ${LIBS_FLAGS}' ${INCL}

mex: clean create mex_nnf mex_disp mex_top mex_vote mex_web

mex_nnf: clean_nnf create
	$(MEX) src/int_single_nnf.cpp -output bin/isnnf -output bin/isnnf
	$(MEX) src/int_k_nnf.cpp -output bin/iknnf -output bin/iknnf
	$(MEX) src/auto_k_nnf.cpp -output bin/autoknnf -output bin/autoknnf

mex_disp: clean_disp create
	$(MEX) src/int_k_disp.cpp -output bin/ikdisp -output bin/ikdisp
	$(MEX) src/float_k_disp.cpp -output bin/fkdisp -output bin/fkdisp

mex_top: clean_top create
	$(MEX) src/int_k_nnf_top.cpp -output bin/iknnf_top -output bin/iknnf_top
	$(MEX) src/float_k_nnf_top.cpp -output bin/fknnf_top -output bin/fknnf_top

mex_vote: clean_vote create
	$(MEX) src/int_vote.cpp -output bin/ivote -output bin/ivote
	$(MEX) src/float_k_disp_vote.cpp -output bin/fkdispvote -output bin/fkdispvote

mex_web: clean_web create
	$(MEX) src/ix_k_nnf.cpp -output bin/ixknnf -output bin/ixknnf

old_mex:
	bash build.sh

clean:
	rm -rf bin
clean_disp:
	rm -rf bin/*disp.mex*
clean_nnf:
	rm -rf bin/*nnf.mex*
clean_test:
	rm -rf bin/test_*
clean_top:
	rm -rf bin/*_top.mex*
clean_vote:
	rm -rf bin/*vote.mex*
clean_web:
	rm -rf bin/*x*nnf.mex*
create:
	mkdir -p bin 2>/dev/null

all: mex
.phony: mex

test: clean_test create test_algo test_int test_float

test_algo: clean_test create
	$(CC) $(INCL) $(subst target,bounds,$(TEST))
	$(CC) $(INCL) $(subst target,scanline,$(TEST))
	$(CC) $(INCL) $(subst target,rng_uniform,$(TEST))
	
test_int: clean_test create
	$(CC) $(INCL) $(subst target,int_single_nnf,$(TEST))
	$(CC) $(INCL) $(subst target,int_k_nnf,$(TEST))
	$(CC) $(INCL) $(subst target,int_vote,$(TEST))
	
test_float: clean_test create
	$(CC) $(INCL) $(subst target,float_k_disp2,$(TEST_WITH_PNG))
	$(CC) $(INCL) $(subst target,float_k_disp,$(TEST_WITH_PNG))
