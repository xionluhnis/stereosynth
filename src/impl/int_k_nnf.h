/* 
 * File:   int_k_nnf.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 3:51 PM
 */

#ifndef INT_K_NNF_H
#define	INT_K_NNF_H

#ifndef USE_MATLAB
#define USE_MATLAB 1
#endif

#include "../algebra.h"
#include "../data/heap.h"
#include "../nnf/patch.h"
#include "../nnf/distance.h"
#include "../nnf/field.h"
#include "../nnf/nnf.h"
#include "../sampling/uniform.h"

#if USE_MATLAB
#include "../matlab.h"
#endif

#include <limits>

namespace pm {

    // distance type
    typedef Distance<Patch2ti, float> DistanceFunc;

    // nearest neighbor field with the k best results
	template <int K>
    struct NearestNeighborField<Patch2ti, float, K> : public Field2D<true> {

        const Image source;
        const Image target;
        const DistanceFunc distFunc;
        const RNG rand;
		const int k;

        NearestNeighborField(const Image &src, const Image &trg, const DistanceFunc d, const RNG r = unif01)
        : Field2D(src.width - Patch2ti::width() + 1, src.height - Patch2ti::width() + 1),
          source(src), target(trg), distFunc(d), rand(r), k(K) {
            data = createEntry<PatchData[K]>("patches");
        }
		
		struct PatchData {
			Patch2ti patch;
			float distance;
            
            PatchData() : patch(), distance(std::numeric_limits<float>::max()) {}
            PatchData(const Patch2ti &p, float d) : patch(p), distance(d) {}
		};
		struct DistanceCompare {
            bool operator ()(const PatchData &p1, const PatchData &p2) const {
                return p1.distance < p2.distance;
            }
		};
        typedef Heap<K, PatchData, DistanceCompare> MaxHeap;

        Entry<PatchData[K]> data;

        float dist(const Point2i &pos, const Patch2ti &q) const {
            const Patch2ti p(pos);
            return distFunc(source, target, p, q);
        }
        inline RNG rng() const {
            return rand;
        }
        inline const Patch2ti &patch(const Point2i &i, int k) const {
            return data.at(i)[k].patch;
        }
        inline const float &distance(const Point2i &i, int k) const {
            // provide the worst distance of all (top)
            return data.at(i)[k].distance;
        }
        inline bool filter(const Point2i &i, const Patch2ti &p) const {
            return false;
        }
        inline bool store(const Point2i &i, const Patch2ti &p, const float &d) {
            return MaxHeap(data.at(i)).insert(PatchData(p, d));
        }
        inline FrameSize targetSize() const {
            return FrameSize(target.width, target.height);
        }

        // --- default initialization ------------------------------------------
        int init(const Point2i &i) {
            PatchData (&p)[K] = data.at(i);
            for(int k = 0; k < K; ++k){
                // initialize with bad data
                p[k].patch = Patch2ti(Point2i(-1, -1));
                p[k].distance = std::numeric_limits<float>::infinity();
            }
			MaxHeap heap(&p[0]);
            int ok = 0;
			for(int k = 0; k < K; ++k){
				// make sure the K patches are different!
                Point2i pos = uniform(
                    rng(),
                    Vec2i(0, 0),
                    Vec2i(target.width - Patch2ti::width(), target.height - Patch2ti::width())
                );
                Patch2ti q(pos);
                PatchData pd(q, dist(i, q));
                // need the distance to insert in the heap
				if(heap.insert(pd)) ++ok;
			}
            return ok;
        }

    #if USE_MATLAB
        void load(const mxArray *d){
            if(mxGetNumberOfElements(d) > 0){
                // transfer data
                const MatXD m(d);
                for(const Point2i &i : *this){
					PatchData (&p)[K] = data.at(i);
					for (int k = 0; k < K; ++k){
						p[k].patch.x = m.read<float>(i.y, i.x, 3 * k + 0);
						p[k].patch.y = m.read<float>(i.y, i.x, 3 * k + 1);
						p[k].distance = m.read<float>(i.y, i.x, 3 * k + 2);
					}
                }
            } else {
                for(const Point2i &i : *this){
                    int k = init(i);
                    while(k < K) {
                        k += init(i);
                    }
                }
            }
        }
        
        void update() {
            for(const Point2i &i : *this){
                PatchData (&p)[K] = data.at(i);
                for (int k = 0; k < K; ++k){
                    p[k].distance = dist(i, p[k].patch);
                }
                // reorder heap
                MaxHeap(&p[0]).build();
            }
        }

        mxArray *save() const {
            mxArray *d = mxCreateMatrix<float>(height, width, 3 * K);
            MatXD m(d);
            for(const Point2i &i : *this){
                const PatchData (&p)[K] = data.at(i);
				for(int k = 0; k < K; ++k){
					m.update(i.y, i.x, 3 * k + 0, float(p[k].patch.x));
					m.update(i.y, i.x, 3 * k + 1, float(p[k].patch.y));
					m.update(i.y, i.x, 3 * k + 2, p[k].distance);
				}
            }
            return d;
        }
    #endif

    };

}

#endif	/* INT_K_NNF_H */

