/* 
 * File:   nnf.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 11:17 AM
 */

#ifndef NNF_H
#define	NNF_H

#include "distance.h"
#include "field.h"
#include "../im/patch.h"

namespace pm {
    
    // distance type
typedef Distance<Patch2ti, float> DistanceFunc;

// nearest neighbor field
template < typename Patch = Patch2ti, typename DistanceValue = float >
struct NearestNeighborField : Field2D<true> {
    
    typedef Patch TargetPatch;
    typedef DistanceValue DistanceType;
    
    const Image source;
    const Image target;
    const DistanceFunc distFunc;
    const RNG rand;
    
    NNF(const Image &src, const Image &trg, const DistanceFunc d, const RNG r = unif01)
    : Field2D(src.width - Patch2ti::width() + 1, src.height - Patch2ti::width() + 1),
      source(src), target(trg), distFunc(d), rand(r) {
        patches = createEntry<Patch2ti>("patches", true); // need to initialize for vtables
		distances = createEntry<float>("distances", false); // no need as we'll overwrite it
    }
	
    TargetPatch &patch(const Point2i &);
    DistanceType &distance(const Point2i &);
    
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
    
}

#endif	/* NNF_H */

