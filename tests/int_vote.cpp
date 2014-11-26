// we do not test with matlab here
#define USE_MATLAB 0

#include "impl/int_single_nnf.h"
#include "impl/int_nnf_container.h"
#include "math/mat.h"
#include "math/vec.h"
#include "voting/weighted_average.h"

#include <cassert>
#include <cmath>
#include <iostream>

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
 * Test the basic integer 1-nnf
 */
int main() {

    Patch2ti::width(7); // set patch size
    assert(Patch2ti::width() == 7 && "Patch width did not update correctly.");
    assert(Patch2tf::width() == 7 && "Patch width isn't spread correctly.");
    seed(timeSeed()); // set rng state

    // create source and target (gradients)
    Image source(100, 100, IM_32FC3);
    for(const auto &i : source){
        auto &v = source.at<Vec3f>(i);
        v[0] = i.x;
        v[1] = i.y;
        v[2] = 0;
    }
    Image target(200, 50, IM_32FC3);
    for(const auto &i : target){
        auto &v = target.at<Vec3f>(i);
        v[0] = i.x;
        v[1] = 0;
        v[2] = i.y;
    }

    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2ti, float>::get(dist::SSD, 3);
    assert(d && "Null distance pointer!");

    // create nnf
    NNF nnf(source, target, d);
    for(const auto &i : nnf){
        nnf.init(i); // random init of patches
    }

    // filter
    Filter filter(7);
    
    // vote result
    VoteOperation<1> op(&nnf, &filter);
    Image img = vote(op, source.channels());
    
    // check that pixels are valid (more to be done!)
    for(const Point2i &i : img){
        const Vec3f &v = img.at<Vec3f>(i);
        for(int i = 0; i < 3; ++i){
            assert(v[i] >= 0 && "Negative pixel value!");
        }
    }
    
    return 0;
}


