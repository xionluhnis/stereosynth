/* 
 * File:   ix_k_nnf.cpp
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 5, 2014, 4:14 PM
 */

#define USE_MATLAB 1

#ifndef KNNF_K
#define KNNF_K 3
#endif

#include "impl/ix_k_nnf.h"
#include "nnf/algorithm.h"
#include "nnf/propagation.h"
#include "nnf/uniformsearch.h"
#include "scanline.h"

typedef unsigned int uint;

using namespace pm;

typedef Patch2tix TargetPatch;
typedef NearestNeighborField<TargetPatch, float, KNNF_K> NNF;
typedef Distance<TargetPatch, float, ImageSet> DistanceFunc;

/**
 * Usage:
 * 
 * [newNNF, conv] = ixknnf( source, {targets}, prevNNF, options )
 */
void mexFunction(int nout, mxArray *out[], int nin, const mxArray *in[]) {
    // checking the input
	if (nin < 2 || nin > 4) {
		mexErrMsgIdAndTxt("MATLAB:nnf:invalidNumInputs",
				"Requires 4 arguments! (#in = %d)", nin);
	}
	// checking the output
	if (nout > 2) {
		mexErrMsgIdAndTxt("MATLAB:nnf:maxlhs",
				"Too many output arguments.");
	}
	
	// options parameter
	mxOptions options(nin >= 4 ? in[3] : mxCreateNothing());
    int numIter = options.integer("iterations", 6);
    int patchSize = options.integer("patch_size", 7);
    uint algo_seed = options.scalar<uint>("rand_seed", timeSeed());
    
    TargetPatch::width(patchSize); // set patch size
    seed(algo_seed); // set rng state
    
    // load source and target
    Image source = mxArrayToImage(in[0]);
    ImageSet targets = mxArrayToImageSet(in[1]);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<TargetPatch, float, ImageSet>::get(dist::SSD, source.channels());
    
    // create nnf (load maybe)
    NNF nnf(source, targets, d);
    nnf.load(nin >= 3 ? in[2] : mxCreateNothing());
    
    // update distance (for external nnf changes)
    if(options.boolean("compute_dist", false)){
        nnf.update();
    }
    
    // create algorithm sequence
    auto seq = Algorithm()  << UniformSearch<TargetPatch, float, KNNF_K>(&nnf)
                            << Propagation<TargetPatch, float, KNNF_K>(&nnf);
    
    // scanline with the sequence of algorithm
    scanline(nnf, numIter, seq);
    
    // save nnf and output it
    if(nout > 0){
        out[0] = nnf.save();
    }
}


