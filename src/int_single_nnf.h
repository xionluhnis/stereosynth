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
#include "nnf/nnf.h"
#include "sampling/uniform.h"

#if USE_MATLAB
#include "matlab.h"
#endif

namespace pm {

    // distance type
    typedef Distance<Patch2ti, float> DistanceFunc;

    // nearest neighbor field
    template<>
    struct NearestNeighborField<Patch2ti, float, 1> : public Field2D<true> {

        const Image source;
        const Image target;
        const DistanceFunc distFunc;
        const RNG rand;

        NearestNeighborField(const Image &src, const Image &trg, const DistanceFunc d, const RNG r = unif01)
        : Field2D(src.width - Patch2ti::width() + 1, src.height - Patch2ti::width() + 1),
          source(src), target(trg), distFunc(d), rand(r) {
            patches = createEntry<Patch2ti>("patches", true); // need to initialize for vtables
            distances = createEntry<float>("distances", false); // no need as we'll overwrite it
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
        inline Patch2ti &patch(const Point2i &i) {
            return patches.at(i);
        }
        inline float &distance(const Point2i &i) {
            return distances.at(i);
        }
        inline FrameSize targetSize() const {
            return FrameSize(target.width, target.height);
        }

        // --- default initialization ------------------------------------------
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
                    distance(i) = m.read<float>(i.y, i.x, 2);
                }
            } else {
                // initialize the field
                for(const Point2i &i : *this){
                    init(i);
                }
            }
        }
        
        void update() {
            for(const Point2i &i : *this){
                distance(i) = dist(i, patch(i));
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

#endif	/* INT_SINGLE_NNF_H */

