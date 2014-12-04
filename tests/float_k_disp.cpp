// we do not test with matlab here
#define USE_MATLAB 0

#ifndef KNNF_K
#define KNNF_K 7
#endif

#include "impl/k_disp.h"
#include "nnf/algorithm.h"
#include "nnf/propagation.h"
#include "nnf/horizontalsearch.h"
#include "nnf/horizontalrandsearch.h"
#include "nnf/randpropagation.h"
#include "scanline.h"

// png++
#include "../libs/pngpp/png.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace pm;

typedef NearestNeighborField<Patch2tf, float, KNNF_K> NNF;
typedef Distance<Patch2tf, float, BilinearMatF> DistanceFunc;

Image load(const png::image<png::rgb_pixel> &img) {
    float max_val = std::numeric_limits<png::byte>::max();
    Image m(img.get_height(), img.get_width(), IM_32FC3);
    for(const auto &i : m){
        auto &v = m.at<Vec3f>(i);
        png::rgb_pixel p = img.get_pixel(i.x, i.y);
        v[0] = p.red    / max_val;
        v[1] = p.green  / max_val;
        v[2] = p.blue   / max_val;
    }
    return m;
}

void checkNNF(NNF &nnf){
    for(const Point2i &i : nnf){
        for(int k = 0; k < KNNF_K; ++k){
            const Patch2tf &p = nnf.patch(i, k);
            float dist = nnf.distance(i, k);
            assert(!std::isinf(dist) && !std::isnan(dist) && "Invalid distance, either infinite or nan.");
            assert(isValid(&nnf, p) && "NNF patch is not valid!");
        }
    }
}

/**
 * Test the basic integer 1-nnf
 */
int main() {
        
    Patch2tf::width(7); // set patch size
    assert(Patch2tf::width() == 7 && "Patch width did not update correctly.");
    assert(Patch2tf::width() == 7 && "Patch width isn't spread correctly.");
    seed(0); // set rng state

    // load source and target
    png::image<png::rgb_pixel> left("tests/data/a.png");
    png::image<png::rgb_pixel> right("tests/data/b.png");
    assert(left.get_width() == right.get_width() && "Different width for left and right!");
    assert(left.get_height() == right.get_height() && "Different height for left and right!");
    
    // our matrices
    BilinearMatF source = load(left);
    BilinearMatF target = load(right);

    // create distance instance
    DistanceFunc d = DistanceFactory<Patch2tf, float, BilinearMatF>::get(dist::SSD, source.channels());
    
    // create nnf
    float maxDY = 0.0f;
    NNF nnf(source, target, d, maxDY);
    for(const Point2i &i : nnf){
        nnf.init(i);
    }
    checkNNF(nnf);
    
    // create algorithm sequence
    SearchRadius<float> search;
    search.radius = std::max(target.width, target.height) * 0.5f;
    search.decreaseFactor = 2.0;
    search.minimum = 7.0f;
    auto seq = Algorithm() << HorizontalSearch<Patch2tf, float, KNNF_K>(&nnf)
                           << HorizontalRandomSearch<Patch2tf, float, KNNF_K>(&nnf, &search, maxDY)
                           << Propagation<Patch2tf, float, KNNF_K>(&nnf)
                           << RandomPropagation<Patch2tf, float, KNNF_K>(&nnf);
    NoOp<Point2i, bool, false> filter;
    DecreasingSearchRadius<float> post(&search);
    
    // scanline with the sequence of algorithm
    scanline(nnf, 3, seq, filter, post);
    
    // check patches
    checkNNF(nnf);
    
    return 0;
}

