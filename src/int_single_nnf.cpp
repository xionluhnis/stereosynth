/* 
 * File:   int_single_nnf.cpp
 * Author: akaspar
 *
 * Created on November 18, 2014, 7:14 AM
 */

#include "algebra.h"
#include "matlab.h"
#include "im/patch.h"
#include "nnf/field.h"

using namespace pm;

struct NNF : Field2D<true> {
    
    const Image source;
    const Image target;
    
    NNF(const Image &src, const Image &trg)
    : Field2D(src.width - Patch2ti::width() + 1, src.height - Patch2ti::width() + 1),
      source(src), target(trg){
        patches = createEntry<Patch>("patches");
		distances = createEntry<float>("distances");
    }
	
	Entry<Patch2ti> patches;
	Entry<float> distances;
};

/**
 * Usage:
 * 
 * [newNNF, conv] = isnnf( source, target, prevNNF, options )
 */
void mexFunction(int nout, mxArray *out[], int nin, const mxArray *in[]) {
    // checking the input
	if (nin != 4) {
		mexErrMsgIdAndTxt("MATLAB:nnf:invalidNumInputs",
				"Requires 4 arguments! (#in = %d)", nin);
	}
	// checking the output
	if (nout > 2) {
		mexErrMsgIdAndTxt("MATLAB:nnf:maxlhs",
				"Too many output arguments.");
	}
	
	// options parameter
	mxOptions options(in[3]);
    int numIters = options.integer("iterations", 6);
    int patchSize = options.integer("patch_size", 7);
    
}

