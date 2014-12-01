/* 
 * File:   int_k_nnf.cpp
 * Author: akaspar
 *
 * Created on November 18, 2014, 7:14 AM
 */

#define USE_MATLAB 1

#ifndef KNNF_K
#define KNNF_K 7
#endif

#include "impl/int_k_nnf.h"
#include "impl/int_single_nnf.h"

using namespace pm;

typedef NearestNeighborField<Patch2ti, float, 1> NNF;
typedef NearestNeighborField<Patch2ti, float, KNNF_K> kNNF;
typedef Distance<Patch2ti, float> DistanceFunc;

/**
 * Usage:
 * 
 * nnf = iknnf_top( source, target, knnf, options )
 */
void mexFunction(int nout, mxArray *out[], int nin, const mxArray *in[]) {
    // checking the input
	if (nin < 3 || nin > 4) {
		mexErrMsgIdAndTxt("MATLAB:nnf:invalidNumInputs",
				"Requires 4 arguments! (#in = %d)", nin);
	}
	// checking the output
	if (nout > 2) {
		mexErrMsgIdAndTxt("MATLAB:nnf:maxlhs",
				"Too many output arguments.");
	}
    
    // load source and target
    Image source = mxArrayToImage(in[0]); // TODO do not allocate data here
    Image target = mxArrayToImage(in[1]);
    
    // implicitly decided patch size
    MatXD nnfMat(in[2]);
    Patch2ti::width(source.width - nnfMat.width + 1);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2ti, float>::get(dist::SSD, source.channels());
    
    // create nnf (load maybe)
    kNNF knnf(source, target, d);
    knnf.load(in[2]);
    
    // update distance (for external nnf changes)
	mxOptions options(nin >= 4 ? in[3] : mxCreateNothing());
    if(options.boolean("compute_dist", false)){
        knnf.update();
    }
    
    // transfer data to 1-nnf
    NNF nnf(source, target, d);
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
        nnf.patches.at(i) = p[bestK].patch;
        nnf.distances.at(i) = bestDist;
    }
    
    // save nnf and output it
    if(nout > 0){
        out[0] = nnf.save();
    }
}
