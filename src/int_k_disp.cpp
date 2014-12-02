/* 
 * File:   int_k_disp.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 2, 2014, 9:41 AM
 */

#define USE_MATLAB 1

#ifndef KNNF_K
#define KNNF_K 7
#endif

#include "impl/int_k_disp.h"
#include "nnf/algorithm.h"
#include "nnf/propagation.h"
#include "nnf/horizontalsearch.h"
#include "nnf/horizontalrandsearch.h"
#include "scanline.h"

typedef unsigned int uint;

using namespace pm;

typedef NearestNeighborField<Patch2ti, float, KNNF_K> NNF;
typedef Distance<Patch2ti, float> DistanceFunc;

/**
 * Usage:
 * 
 * [newNNF, conv] = ikdisp( left, right, prevNNF, options )
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
    int maxDY = options.integer("max_dy", 5);
    
    Patch2ti::width(patchSize); // set patch size
    seed(algo_seed); // set rng state
    
    // load source and target
    Image source = mxArrayToImage(in[0]);
    Image target = mxArrayToImage(in[1]);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2ti, float>::get(dist::SSD, source.channels());
    
    // create nnf (load maybe)
    NNF nnf(source, target, d, maxDY);
    nnf.load(nin >= 3 ? in[2] : mxCreateNothing());
    
    // update distance (for external nnf changes)
    if(options.boolean("compute_dist", false)){
        nnf.update();
    }
    
    // create algorithm sequence
    SearchRadius<int> search;
    search.radius = options.integer("search_radius", std::max(target.width, target.height) / 2);
    search.decreaseFactor = options.scalar<double>("search_decr_factor", 2.0);
    search.minimum = options.integer("search_min_radius", patchSize);
    auto seq = Algorithm() << HorizontalSearch<Patch2ti, float, KNNF_K>(&nnf)
                           << HorizontalRandomSearch<Patch2ti, float, KNNF_K>(&nnf, &search, maxDY)
                           << Propagation<Patch2ti, float, KNNF_K>(&nnf);
    NoOp<Point2i, false> filter;
    DecreasingSearchRadius<int> post(&search);
    
    // scanline with the sequence of algorithm
    scanline(nnf, numIter, seq, filter, post);
    
    // save nnf and output it
    if(nout > 0){
        out[0] = nnf.save();
    }
}

