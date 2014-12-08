/* 
 * File:   ix_k_nnf.cpp
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 5, 2014, 4:14 PM
 */

#define USE_MATLAB 1

#ifndef KNNF_K
#define KNNF_K 7
#endif

#include "impl/ix_k_nnf.h"
#include "nnf/algorithm.h"
#include "nnf/propagation.h"
#include "nnf/uniformsearch.h"
#include "scanline.h"

typedef unsigned int uint;

using namespace pm;

typedef Patch2tix TargetPatch;
typedef NearestNeighborField<TargetPatch, float, 1> NNF;
typedef NearestNeighborField<TargetPatch, float, KNNF_K> kNNF;
typedef Distance<TargetPatch, float, ImageSet> DistanceFunc;

/**
 * Usage:
 * 
 * nnf = ixknnf_top( source, {targets}, knnf, options )
 */
void mexFunction(int nout, mxArray *out[], int nin, const mxArray *in[]) {
    // checking the input
	if (nin < 3 || nin > 4) {
		mexErrMsgIdAndTxt("MATLAB:nnf:invalidNumInputs",
				"Requires 4 arguments! (#in = %d)", nin);
	}
	// checking the output
	if (nout > 1) {
		mexErrMsgIdAndTxt("MATLAB:nnf:maxlhs",
				"Too many output arguments.");
	}
	
	// options parameter
	mxOptions options(nin >= 4 ? in[3] : mxCreateNothing());
    int patchSize = options.integer("patch_size", 7);
    uint algo_seed = options.scalar<uint>("rand_seed", timeSeed());
    
    TargetPatch::width(patchSize); // set patch size
    seed(algo_seed); // set rng state
    
    // load source and target
    Image source = mxArrayToImage(in[0]);
    ImageSet targets = mxArrayToImageSet(in[1]);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<TargetPatch, float, ImageSet>::get(dist::SSD, source.channels());
    
    // create nnf (load it)
    kNNF knnf(source, targets, d);
    knnf.load(in[2]);
    
    // update distance (for external nnf changes)
    if(options.boolean("compute_dist", false)){
        knnf.update();
    }
    
    // transfer data to 1-nnf
    NNF nnf(source, targets, d);
    for(const Point2i &i : knnf){
        typename kNNF::PatchData (&p)[KNNF_K] = knnf.data.at(i);
        int bestK = 0;
        float bestDist = p[0].distance;
        for(int k = 1; k < KNNF_K; ++k){
            if(p[k].distance < bestDist){
                bestDist = p[k].distance;
                bestK = k;
            }
        }
        nnf.store(i, p[bestK].patch, bestDist);
    }
    
    // save nnf and output it
    if(nout > 0){
        out[0] = nnf.save();
    }
}



