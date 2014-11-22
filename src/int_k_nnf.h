/* 
 * File:   int_k_nnf.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 3:51 PM
 */

#ifndef INT_K_NNF_H
#define	INT_K_NNF_H

#ifndef USE_MATLAB
#define USE_MATLAB 1
#endif

#include "algebra.h"
#include "im/patch.h"
#include "nnf/distance.h"
#include "nnf/field.h"
#include "nnf/nnf.h"
#include "sampling/uniform.h"

#if USE_MATLAB
#include "matlab.h"
#endif

#include <queue>

namespace pm {

    // distance type
    typedef Distance<Patch2ti, float> DistanceFunc;

    // nearest neighbor field with the k best results
    struct kNNF: public Field2D<true> {

        const Image source;
        const Image target;
        const DistanceFunc distFunc;
        const RNG rand;

        kNNF(const Image &src, const Image &trg, const DistanceFunc d, const RNG r = unif01)
        : Field2D(src.width - Patch2ti::width() + 1, src.height - Patch2ti::width() + 1),
          source(src), target(trg), distFunc(d), rand(r) {
            patches = createEntry<Patch2ti>("patches", true); // need to initialize for vtables
            distances = createEntry<float>("distances", false); // no need as we'll overwrite it
            
            std::priority_queue<double> q;
        }

        Entry<Patch2ti> patches;
        Entry<float> distances;

        float dist(const Point2i &pos, const Patch2ti &q) const {
            const Patch2ti p(pos);
            return distFunc(source, target, p, q);
        }

        inline RNG rng() const {
            return rand;
        }
        inline FrameSize targetSize() const {
            return FrameSize(target.width, target.height);
        }

        // --- default initialization ------------------------------------------
        void init(const Point2i &i) {
            Patch2ti &p = patches.at(i);
            Point2i pos = uniform(
                rng(),
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

#endif	/* INT_K_NNF_H */

