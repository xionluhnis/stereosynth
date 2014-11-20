/* 
 * File:   int_single_nnf.cpp
 * Author: akaspar
 *
 * Created on November 18, 2014, 7:14 AM
 */

#define USE_MATLAB 1

#include "int_single_nnf.h"
#include "scanline.h"

typedef unsigned int uint;

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
    int numIter = options.integer("iterations", 6);
    int patchSize = options.integer("patch_size", 7);
    uint algo_seed = options.scalar<uint>("rand_seed", timeSeed());
    
    Patch2ti::width(patchSize); // set patch size
    seed(algo_seed); // set rng state
    
    // load source and target
    Image source = mxArrayToImage(in[0]);
    Image target = mxArrayToImage(in[1]);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2ti, float>::get(dist::SSD, source.channels());
    
    // create nnf (load maybe)
    NNF nnf(source, target, d);
    nnf.load(in[2]);
    
    // create algorithm sequence
    auto seq = Algorithm() << UniformSearch(&nnf) << Propagation(&nnf);
    
    // scanline with the sequence of algorithm
    scanline(nnf, numIter, seq);
    
    // save nnf and output it
    if(nout > 0){
        out[0] = nnf.save();
    }
}

