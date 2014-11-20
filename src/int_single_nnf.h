/* 
 * File:   int_single_nnf.h
 * Author: akaspar
 *
 * Created on November 19, 2014, 5:04 PM
 */

#ifndef INT_SINGLE_NNF_H
#define	INT_SINGLE_NNF_H

#ifndef USE_MATLAB
#define USE_MATLAB 1
#endif

#include "algebra.h"
#include "im/patch.h"
#include "nnf/distance.h"
#include "nnf/field.h"
#include "sampling/uniform.h"

#if USE_MATLAB
#include "matlab.h"
#endif

#include <functional>

using namespace pm;

// distance type
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
        patches = createEntry<Patch2ti>("patches");
		distances = createEntry<float>("distances");
    }
	
	Entry<Patch2ti> patches;
	Entry<float> distances;
    
    float dist(const Point2i &pos, const Patch2ti &q) const {
        const Patch2ti p(pos);
        return distFunc(source, target, p, q);
    }
    
    void init(const Point2i &i) {
        Patch2ti &p = patches.at(i);
        Point2i pos = uniform(
            rand,
            Vec2i(0, 0),
            Vec2i(target.width - Patch2ti::width(), target.height - Patch2ti::width())
        );
        p = Patch2ti(pos); // assign position
        distances.at(i) = dist(i, p);
    }
    
#if USE_MATLAB
    void load(const mxArray *data){
        if(mxGetNumberOfElements(data) > 0){
            // transfer data
            const MatXD m(data);
            for(const Point2i &i : *this){
                Patch2ti &p = patches.at(i);
                p.x = m.read<float>(i.y, i.x, 0);
                p.y = m.read<float>(i.y, i.x, 1);
                distances.at(i) = m.read<float>(i.y, i.x, 2);
            }
        } else {
            // initialize the field
            for(const Point2i &i : *this){
                init(i);
            }
        }
    }
    
    mxArray *save() const {
        mxArray *data = mxCreateMatrix<float>(height, width, 3);
        MatXD m(data);
        for(const Point2i &i : *this){
            const Patch2ti &p = patches.at(i);
            m.update(i.y, i.x, 0, float(p.x));
            m.update(i.y, i.x, 1, float(p.y));
            m.update(i.y, i.x, 2, distances.at(i));
        }
        return data;
    }
#endif
    
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
            Patch2ti q = nnf->patches.at(j).transform(delta); // transform delta in patch => new position => new patch
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

struct Algorithm {
    typedef std::function<bool(const Point2i &, bool)> AlgorithmPart;
    bool operator()(const Point2i &i, bool rev) {
        bool res = false;
        for(AlgorithmPart &p : seq){
            res |= p(i, rev);
        }
        return res;
    }
    
    Algorithm(){}
    
    Algorithm &operator <<(AlgorithmPart p){
        seq.push_back(p);
        return *this;
    }
private:
    std::vector<AlgorithmPart> seq;
};

#endif	/* INT_SINGLE_NNF_H */

