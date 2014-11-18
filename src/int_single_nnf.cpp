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

template< typename Patch >
struct NNF : Field2D {
    
    const Image source;
    const Image target;
    
    NNF(const Image &src, const Image &trg)
    : Field2D(src.width - Patch::width() + 1, src.height - Patch::width() + 1),
      source(src), target(trg){
        
    }
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
	if (!mxIsStruct(in[OPTIONS_IDX])) {
		mexErrMsgIdAndTxt("MATLAB:nnf:invalidOptions",
				"Options must be put in a structure.");
	}
}

