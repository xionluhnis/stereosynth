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
#include "data/heap.h"
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

    // nearest neighbor field with the k best results
	template <int K = 7>
    struct kNNF: public Field2D<true> {

        const Image source;
        const Image target;
        const DistanceFunc distFunc;
        const RNG rand;
		const int k;

        kNNF(const Image &src, const Image &trg, const DistanceFunc d, const RNG r = unif01)
        : Field2D(src.width - Patch2ti::width() + 1, src.height - Patch2ti::width() + 1),
          source(src), target(trg), distFunc(d), rand(r), k(K) {
            patches = createEntry<PatchData[K]>("patches");
        }
		
		struct PatchData {
			Patch2ti patch;
			float distance;
		};
		struct DistanceCompare(const PatchData &p1, const PatchData &p2) {
			return p1.distance < p2.distance;
		}

        Entry<PatchData[K]> data;

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
            PatchData &p[K] = data.at(i);
			Heap<K, PatchData> heap(&p[0]);
			for(int i = 0; i < K; ++i){
				// TODO make sure the K patches are different
				Point2i pos = uniform(
					rng(),
					Vec2i(0, 0),
					Vec2i(target.width - Patch2ti::width(), target.height - Patch2ti::width())
				);
				heap.insert(Patch2ti(pos));
			}
        }

    #if USE_MATLAB
        void load(const mxArray *data){
            if(mxGetNumberOfElements(data) > 0){
                // transfer data
                const MatXD m(data);
                for(const Point2i &i : *this){
					PatchData &p[K] = data.at(i);
					for (int j = K-1; j >= 0; --j){
						p[j].patch.x = m.read<float>(i.y, i.x, 3 * j + 0);
						p[j].patch.y = m.read<float>(i.y, i.x, 3 * j + 1);
						p[j].distance = m.read<float>(i.y, i.x, 3 * j + 2);
					}
                }
            } else {
                // initialize the field
                for(const Point2i &i : *this){
                    init(i);
                }
            }
        }

        mxArray *save() const {
            mxArray *data = mxCreateMatrix<float>(height, width, 3 * K);
            MatXD m(data);
            for(const Point2i &i : *this){
                const PatchData &p[K] = data.at(i);
				for(int j = K-1; j >= 0; --j){
					m.update(i.y, i.x, 0, float(p[j].patch.x));
					m.update(i.y, i.x, 1, float(p[j].patch.y));
					m.update(i.y, i.x, 2, p[j].distance);
				}
            }
            return data;
        }
    #endif

    };

}

#endif	/* INT_K_NNF_H */

