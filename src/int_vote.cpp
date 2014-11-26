/* 
 * File:   int_k_nnf.cpp
 * Author: akaspar
 *
 * Created on November 18, 2014, 7:14 AM
 */

#define USE_MATLAB 1

#include "impl/int_single_nnf.h"
#include "impl/int_nnf_container.h"
#include "math/bounds.h"
#include "voting/weighted_average.h"
#include "matlab.h"

using namespace pm;

typedef NearestNeighborField<Patch2ti, float, 1> NNF;
typedef Distance<Patch2ti, float> DistanceFunc;

namespace pm {

    template <int channels = 1>
    struct VoteOperation {
        
        typedef VoteOperation<channels + 1> Next;
        
        Image compute() const{
            PixelContainer<channels, Patch2ti, float> data(nnf);
            return weighted_average(data, *filter);
        }

        VoteOperation(const VoteOperation<channels-1> &v) :  nnf(v.nnf), filter(v.filter) {}
        VoteOperation(NNF *n, Filter *f) : nnf(n), filter(f) {}

        NNF *nnf;
        Filter *filter;
    };

}

/**
 * Usage:
 * 
 * img = ivote( source, target, knnf, options )
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
    Image source = mxArrayToImage(in[0]);
    Image target = mxArrayToImage(in[1]);
    
    // implicitly decided patch size
    MatXD nnfMat(in[2]);
    int patchSize = source.width - nnfMat.width + 1;
    Patch2ti::width(patchSize);
    
    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2ti, float>::get(dist::SSD, source.channels());
    
    // create nnf (load maybe)
    NNF nnf(source, target, d);
    nnf.load(in[2]);
    
    // update distance (for external nnf changes)
	mxOptions options(nin >= 4 ? in[3] : mxCreateNothing());
    if(options.boolean("compute_dist", false)){
        nnf.update();
    }
    
    // filter
    Filter filter(patchSize);
    if(options.has("vote_filter")){
        filter.weight = options.vector("vote_filter", 1.0f, patchSize * patchSize);
    }
    
    // vote result
    VoteOperation<1> op(&nnf, &filter);
    Image img = vote(op, source.channels());
    if(nout > 0){
        out[0] = mxImageToArray(img);
    }
}

