/* 
 * File:   int_k_nnf.cpp
 * Author: akaspar
 *
 * Created on November 18, 2014, 7:14 AM
 */

#define USE_MATLAB 1

#include "int_single_nnf.h"
#include "voting/weighted_average.h"

using namespace pm;

typedef NearestNeighborField<Patch2ti, float, 1> NNF;
typedef Distance<Patch2ti, float> DistanceFunc;

template < int numChannels >
struct PixelContainer<numChannels, Patch2ti, float> {
    typedef Vec<float, numChannels> vec;
    typedef Point2i point;

    enum {
        channels = numChannels
    };

    Frame2D<Point2i, true> frame() const {
        return Frame2D<Point2i, true>(nnf->targetSize());
    }
    const vec &pixel(const point &p) const {
        return nnf->target.at<vec>(p);
    }
    SubFrame2D<Point2i, true> overlap(const point &p) const {
        Bounds2i frame(Vec2i(0, 0), Vec2i(nnf->source.width, nnf->source.height));
        Bounds2i zone = frame & Bounds2i(p - Vec2i(Patch2ti::width() - 1, Patch2ti::width() - 1), p);
        return SubFrame2D<Point2i, true>(zone.min, zone.max);
    }
    const Patch2ti &patch(const point &i) const {
        return nnf->patches.at(i);
    }
    
    PixelContainer(NNF *n) : nnf(n) {}
private:
    NNF *nnf;
};

struct VoteOperation {
    template <int channels>
    Image compute(){
        PixelContainer<channels, Patch2ti, float> data(nnf);
        return weighted_average(data, filter);
    }
    
    VoteOperation(NNF *n, const Filter &f) : nnf(n), filter(f) {}
    
private:
    NNF *nnf;
    Filter filter;
};

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
    kNNF nnf(source, target, d);
    nnf.load(in[2]);
    
    // update distance (for external nnf changes)
	mxOptions options(nin >= 4 ? in[3] : mxCreateNothing());
    if(options.boolean("compute_dist", false)){
        nnf.update();
    }
    
    // filter
    Filter filter(patchSize);
    if(options.has("vote_filter")){
        filter.weight = options.vector("vote_filter", 1.0f);
    }
    
    // vote result
    VoteOperation vote(&nnf, filter);
    Image img = compute(vote, source.channels());
    if(nout > 0){
        out[0] = img;
    }
}

