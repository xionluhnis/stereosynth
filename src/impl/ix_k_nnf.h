/* 
 * File:   ix_k_nnf.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 5, 2014, 3:31 PM
 */

#ifndef IX_K_NNF_H
#define	IX_K_NNF_H

#ifndef USE_MATLAB
#define USE_MATLAB 1
#endif

#define ONLY_K_NNF 1

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
    typedef Distance<Patch2tix, float, ImageSet> DistanceFunc;

    // nearest neighbor field with the k best results
	template <int K>
    struct NearestNeighborField<Patch2tix, float, K> : public Field2D<true> {
        
        typedef Patch2tix TargetPatch;
        typedef typename Patch2tix::SourcePatch SourcePatch;

        const Image source;
        const ImageSet targets;
        const DistanceFunc distFunc;
        const RNG rand;
		const int k;

        NearestNeighborField(const Image &src, const ImageSet &trg, const DistanceFunc d, const RNG r = unif01)
        : Field2D(src.width - TargetPatch::width() + 1, src.height - TargetPatch::width() + 1),
          source(src), targets(trg), distFunc(d), rand(r), k(K) {
            data = createEntry<PatchData[K]>("patches");
        }
		
		struct PatchData {
			Patch2tix patch;
			float distance;
            
            PatchData() : patch(), distance(std::numeric_limits<float>::max()) {}
            PatchData(const TargetPatch &p, float d) : patch(p), distance(d) {}
		};
		struct DistanceCompare {
            bool operator ()(const PatchData &p1, const PatchData &p2) const {
                return p1.distance < p2.distance;
            }
		};
        typedef Heap<K, PatchData, DistanceCompare> MaxHeap;

        Entry<PatchData[K]> data;

        float dist(const Point2i &pos, const TargetPatch &q) const {
            const SourcePatch p(pos);
            return distFunc(source, targets, p, q);
        }
        inline RNG rng() const {
            return rand;
        }
        inline const TargetPatch &patch(const Point2i &i, int k) const {
            const PatchData (& p)[K] = data.at(i);
            assert(p[k].patch.index < targets.size() && "Patch out of image index bounds");
            return p[k].patch;
        }
        inline const float &distance(const Point2i &i, int k) const {
            // provide the worst distance of all (top)
            return data.at(i)[k].distance;
        }
        inline bool filter(const Point2i &i, const TargetPatch &p) const {
            return false;
        }
        inline bool store(const Point2i &i, const TargetPatch &p, const float &d) {
            return MaxHeap(data.at(i)).insert(PatchData(p, d));
        }
        inline FrameSize targetSize(size_t n) const {
            return FrameSize(targets[n].width, targets[n].height);
        }
        inline size_t targetCount() const {
            return targets.size();
        }

        // --- default initialization ------------------------------------------
        int init(const Point2i &i) {
            PatchData (&p)[K] = data.at(i);
            for(int k = 0; k < K; ++k){
                // initialize with bad data
                p[k].patch = TargetPatch(Point2ix(-1, -1, -1));
                p[k].distance = std::numeric_limits<float>::infinity();
            }
			MaxHeap heap(&p[0]);
            int ok = 0;
			for(int k = 0; k < K; ++k){
                // choose image to sample from
                size_t z = uniform<size_t>(rng(), 0, targets.size() - 1);
				// make sure the K patches are different!
                Point2i pos = uniform(
                    rng(),
                    Vec2i(0, 0),
                    Vec2i(targets[z].width - TargetPatch::width(), targets[z].height - TargetPatch::width())
                );
                TargetPatch q(pos, z);
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
						p[k].patch.x = m.read<float>(i.y, i.x, 4 * k + 0);
						p[k].patch.y = m.read<float>(i.y, i.x, 4 * k + 1);
						p[k].patch.z = m.read<float>(i.y, i.x, 4 * k + 2);
						p[k].distance = m.read<float>(i.y, i.x, 4 * k + 3);
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
                    assert(p[k].patch.index >= 0 && p[k].patch.index < targets.size() && "Update with patch out of image set bounds");
                    p[k].distance = dist(i, p[k].patch);
                }
                // reorder heap
                MaxHeap(&p[0]).build();
            }
        }

        mxArray *save() const {
            mxArray *d = mxCreateMatrix<float>(height, width, 4 * K);
            MatXD m(d);
            for(const Point2i &i : *this){
                const PatchData (&p)[K] = data.at(i);
				for(int k = 0; k < K; ++k){
					m.update(i.y, i.x, 4 * k + 0, float(p[k].patch.x));
					m.update(i.y, i.x, 4 * k + 1, float(p[k].patch.y));
                    m.update(i.y, i.x, 4 * k + 2, float(p[k].patch.z));
					m.update(i.y, i.x, 4 * k + 3, p[k].distance);
				}
            }
            return d;
        }
    #endif

    };

}

#endif	/* IX_K_NNF_H */

