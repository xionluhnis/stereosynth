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

#include "impl/k_disp.h"
#include "nnf/algorithm.h"
#include "nnf/propagation.h"
#include "nnf/horizontalsearch.h"
#include "nnf/horizontalrandsearch.h"
#include "scanline.h"

typedef unsigned int uint;

using namespace pm;

typedef NearestNeighborField<Patch2tf, float, KNNF_K> NNF;
typedef Distance<Patch2tf, float, BilinearMatF> DistanceFunc;

/**
 * Usage:
 * 
 * [newNNF, conv] = fkdisp( left, right, prevNNF, options )
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
    
    Patch2tf::width(patchSize); // set patch size
    seed(algo_seed); // set rng state
    
    // load source and target
    BilinearMatF source = mxArrayToImage(in[0]);
    BilinearMatF target = mxArrayToImage(in[1]);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2tf, float, BilinearMatF>::get(dist::SSD, source.channels());
    
    // create nnf (load maybe)
    NNF nnf(source, target, d, maxDY);
    nnf.load(nin >= 3 ? in[2] : mxCreateNothing());
    
    // update distance (for external nnf changes)
    if(options.boolean("compute_dist", false)){
        nnf.update();
    }
    
    // create algorithm sequence
    SearchRadius<float> search;
    search.radius = options.scalar<float>("search_radius", std::max(target.width, target.height) / 2.0f);
    search.decreaseFactor = options.scalar<double>("search_decr_factor", 2.0);
    search.minimum = options.scalar<float>("search_min_radius", float(patchSize));
    auto seq = Algorithm() << HorizontalSearch<Patch2tf, float, KNNF_K>(&nnf)
                           << HorizontalRandomSearch<Patch2tf, float, KNNF_K>(&nnf, &search, maxDY)
                           << Propagation<Patch2tf, float, KNNF_K>(&nnf);
    NoOp<Point2i, false> filter;
    DecreasingSearchRadius<float> post(&search);
    
    // scanline with the sequence of algorithm
    scanline(nnf, numIter, seq, filter, post);
    
    // save nnf and output it
    if(nout > 0){
        out[0] = nnf.save();
    }
}

