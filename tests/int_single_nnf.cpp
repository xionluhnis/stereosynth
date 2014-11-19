// we do not test with matlab here
#define USE_MATLAB 0

#include "int_single_nnf.h"

#include <cassert>
#include <cmath>
#include <iostream>

/**
 * Test the basic integer 1-nnf
 */
int main() {

    // we do it multiple times for different seeds
    // for(int i = 0; i < 100; ++i){
        
        Patch2ti::width(7); // set patch size
        seed(timeSeed() + i); // set rng state

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

        // create nnf
        NNF nnf(source, target, d);
        for(const auto &i : nnf){
            nnf.init(i); // random init of patches
        }

        // create algorithm sequence
        auto seq = algoSeq<UniformSearch, Propagation>(&nnf);

        // scanline with the sequence of algorithm
        scanline(nnf, 3, seq);
    
        // TODO 
    // }
    
    return 0;
}

