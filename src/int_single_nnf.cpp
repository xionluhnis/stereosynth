/* 
 * File:   int_single_nnf.cpp
 * Author: akaspar
 *
 * Created on November 18, 2014, 7:14 AM
 */

#include "algebra.h"
#include "matlab.h"
#include "im/patch.h"
#include "nnf/distance.h"
#include "nnf/field.h"

using namespace pm;

typedef Distance<Patch2ti, float> DistanceFunc;

// nearest neighbor field
struct NNF : Field2D<true> {
    
    const Image source;
    const Image target;
    const DistanceFunc distFunc;
    const RNG rand;
    
    NNF(const Image &src, const Image &trg, const DistanceFunc d, const RNG r = unif01)
    : Field2D(src.width - Patch2ti::width() + 1, src.height - Patch2ti::width() + 1),
      source(src), target(trg), distFunc(d), rand(r) {
        patches = createEntry<Patch>("patches");
		distances = createEntry<float>("distances");
    }
	
	Entry<Patch2ti> patches;
	Entry<float> distances;
    
    float dist(const Point2i &pos, const Patch2ti &q) const {
        const Patch2ti p(pos);
        return distFunc(source, target, p, q);
    }
    
    void load(const mxArray *data){
        
    }
    
    mxArray *save() {
        
    }
};

struct NoAlgorithm {
    NoAlgorithm(void*){}
};

class TryPatch {
public:
    
    bool tryPatch(const Point2i &i, const Patch2ti q) {
        Patch2ti &p = nnf->patches.at(i);
        // if it's the same patch, too bad
        if(p == q){
            return false;
        }
        // compute distance for the new patch
        float newDist = nnf->dist(i, q);
        float &curDist = nnf->distances.at(i);
        
        if(newDist < curDist){
            p = q; // replace patch
            curDist = newDist; // store new distance
            return true;
        }
        return false;
    }
    
    TryPatch(NNF *n) : nnf(n){}
protected:
    NNF *nnf;
};

class UniformSearch : public TryPatch{
public:
    
    bool operator()(const Point2i &i, bool) {
        // uniformly sample a position for the new patch
        Point2i q = uniform(
            nnf->rand,
            Vec2i(0, 0),
            Vec2i(nnf->target.width - Patch2ti::width(), nnf->target.height - Patch2ti::width())
        );
        return tryPatch(i, Patch2ti(q));
    }
    
    UniformSearch(NNF *nnf) : TryPatch(nnf){}
};

class Propagation : public TryPatch {
public:
    
    bool tryDelta(const Point2i &i, const Point2i &delta) {
        Point2i j = i - delta;
        if(nnf->contains(j)){
            Patch2ti q = nnf->patches.at(j) * dx; // transform delta in patch => new position => new patch
            return tryPatch(i, q);
        }
        return false;
    }
    
    bool operator()(const Point2i &i, bool rev) {
        // direction for deltas
        int d = rev ? -1 : 1;
        // two propagation tentatives
        bool res = false;
        res |= tryDelta(i, Point2i(d, 0)); // dx
        res |= tryDelta(i, Point2i(0, d)); // dy
        return res;
    }
    
    Propagation(NNF *nnf) : TryPatch(nnf){}
};

template < typename Algorithm, typename NextAlgorithm = NoAlgorithm >
struct AlgoSeq {
    bool operator()(const Point2i &i, bool rev) {
        bool res = algorithm(i, rev);
        res |= next(i, rev);
        return res;
    }
    
    AlgoSeq(NNF *nnf) : algorithm(nnf), next(nnf){
    }
    AlgoSeq(Algorithm algo, NextAlgorithm n) : algorithm(algo), next(n) {}
private:
    Algorithm algorithm;
    NextAlgorithm next;
};

template < typename Algorithm >
bool AlgoSeq<Algorithm, NoAlgorithm>::operator()(const Point2i &i, bool rev) {
    return algorithm(i, rev); // no need to check the next algorithm
}

template < typename FirstAlgo, typename... AlgoSeq>
auto algoSeq(NNF *nnf){
    auto next = algoSeq<AlgoSeq>(nnf);
    Algorithm algo(nnf);
    return AlgoSeq(algo, next);
}

template< typename Algorithm >
AlgoSeq<Algorithm, NoAlgorithm> algoSeq(NNF *nnf) {
    Algorithm algo(nnf);
    NoAlgorithm end(nnf);
    return AlgoSeq<Algorithm, NoAlgorithm>(algo, end);
}

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
    auto seq = algoSeq<UniformSearch, Propagation>(&nnf);
    
    // scanline with the sequence of algorithm
    scanline(nnf, numIter, seq);
    
    // save nnf and output it
    if(nout > 0){
        nout[0] = nnf.save();
    }
}

