/* 
 * File:   float_k_disp_vote.cpp
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 4, 2014, 7:14 AM
 */

#define USE_MATLAB 1

#ifndef KNNF_K
#define KNNF_K 7
#endif

#include "impl/k_disp.h"
#include "impl/k_disp_container.h"
#include "math/bounds.h"
#include "voting/disparity_vote.h"
#include "matlab.h"

using namespace pm;

typedef NearestNeighborField<Patch2tf, float, KNNF_K> NNF;
typedef Distance<Patch2tf, float, BilinearMatF> DistanceFunc;

namespace pm {

    template <int channels = 1>
    struct VoteOperation {
        
        typedef VoteOperation<channels + 1> Next;
        
        Image compute() const{
            PixelContainer<channels, Patch2tf, float, KNNF_K> data(nnf);
            return disparity_vote(data, *filter, *nnf, prctile);
        }

        VoteOperation(const VoteOperation<channels-1> &v) :  nnf(v.nnf), filter(v.filter), prctile(v.prctile) {}
        VoteOperation(NNF *n, Filter *f, float p) : nnf(n), filter(f), prctile(p) {}

        NNF *nnf;
        Filter *filter;
        float prctile;
    };

}

/**
 * Usage:
 * 
 * uv = fkdisp_vote( source, target, knnf, options )
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
    
    // load source and target
    BilinearMatF source = mxArrayToImage(in[0]);
    BilinearMatF target = mxArrayToImage(in[1]);
    
    // implicitly decided patch size
    MatXD nnfMat(in[2]);
    int patchSize = source.width - nnfMat.width + 1;
    Patch2tf::width(patchSize);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2tf, float, BilinearMatF>::get(dist::SSD, source.channels());
    
    // create nnf (load maybe)
    NNF nnf(source, target, d);
    nnf.load(in[2]);
    
    // update distance (for external nnf changes)
	mxOptions options(nin >= 4 ? in[3] : mxCreateNothing());
    assert(options.integer("patch_size", patchSize) == patchSize && "The patch size does not fit image and nnf dimensions.");
    if(options.boolean("compute_dist", false)){
        nnf.update();
    }
    
    // filter
    Filter filter(patchSize);
    if(options.has("vote_filter")){
        filter.weight = options.vector("vote_filter", 1.0f, patchSize * patchSize);
    }
    
    // vote result
    VoteOperation<1> op(&nnf, &filter, options.scalar<float>("disp_prctile", 0.975));
    Image img = vote(op, source.channels());
    if(nout > 0){
        out[0] = mxImageToArray(img);
    }
}


