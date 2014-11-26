// we do not test with matlab here
#define USE_MATLAB 0

#include "impl/int_k_nnf.h"
#include "nnf/algorithm.h"
#include "nnf/propagation.h"
#include "nnf/uniformsearch.h"
#include "scanline.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace pm;

typedef NearestNeighborField<Patch2ti, float, 7> NNF;
typedef Distance<Patch2ti, float> DistanceFunc;

/**
 * Test the basic integer 1-nnf
 */
int main() {

    // we do it multiple times for different seeds
    int i = 0;
    // for(int i = 0; i < 100; ++i){
        
        Patch2ti::width(7); // set patch size
		assert(Patch2ti::width() == 7 && "Patch width did not update correctly.");
		assert(Patch2tf::width() == 7 && "Patch width isn't spread correctly.");
        seed(0 /*timeSeed()*/ + i); // set rng state

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

        // create algorithm sequence
        auto seq = Algorithm() << UniformSearch<Patch2ti, float, 7>(&nnf) << Propagation<Patch2ti, float, 7>(&nnf);

        // scanline with the sequence of algorithm
        scanline(nnf, 3, seq);
    
        // TODO 
    // }
    
    return 0;
}

