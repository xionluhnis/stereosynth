/* 
 * File:   int_k_disp.h
 * Author: akaspar
 *
 * Created on December 2, 2014, 10:15 AM
 */

#ifndef INT_K_DISP_H
#define	INT_K_DISP_H

#ifndef USE_MATLAB
#define USE_MATLAB 1
#endif

#include "../algebra.h"
#include "../data/heap.h"
#include "../im/bilinear.h"
#include "../im/patch.h"
#include "../nnf/distance.h"
#include "../nnf/field.h"
#include "../nnf/nnf.h"
#include "../sampling/uniform.h"

#if USE_MATLAB
#include "../matlab.h"
#endif

#include <limits>
#include <type_traits>

namespace pm {
    
    template < typename S >
    struct mat_impl {
        typedef typename std::conditional<std::is_floating_point<S>::value, BilinearMat<S>, Mat>::type type;
    };

    // disparity field with the k best results
	template <typename S, int K>
    struct NearestNeighborField<BasicPatch<S>, float, K> : public Field2D<true> {
        typedef BasicPatch<S> TargetPatch;
        typedef typename BasicPatch<S>::point TargetPoint;
        typedef typename TargetPatch::SourcePatch SourcePatch;
        typedef typename mat_impl<S>::type ImageType;
        typedef Distance<TargetPatch, float, ImageType> DistanceFunc;

        const ImageType source;
        const ImageType target;
        const DistanceFunc distFunc;
        const RNG rand;
		const int k;
        const int maxDY;

        NearestNeighborField(const ImageType &src, const ImageType &trg, const DistanceFunc d, int dy = 5, const RNG r = unif01)
        : Field2D(src.width - TargetPatch::width() + 1, src.height - TargetPatch::width() + 1),
          source(src), target(trg), distFunc(d), rand(r), k(K), maxDY(dy) {
            data = createEntry<PatchData[K]>("patches");
        }
		
		struct PatchData {
			TargetPatch patch;
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
            return distFunc(source, target, p, q);
        }
        inline RNG rng() const {
            return rand;
        }
        inline const TargetPatch &patch(const Point2i &i, int k) const {
            return data.at(i)[k].patch;
        }
        inline const float &distance(const Point2i &i, int k) const {
            // provide the worst distance of all (top)
            return data.at(i)[k].distance;
        }
        inline bool filter(const Point2i &i, const TargetPatch &p) const {
            return (i - p).abs().y > maxDY; // filter patches deviating too much vertically
        }
        inline bool store(const Point2i &i, const TargetPatch &p, const float &d) {
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
                p[k].patch = TargetPatch(TargetPoint(-1, -1));
                p[k].distance = std::numeric_limits<float>::infinity();
            }
			MaxHeap heap(&p[0]);
            // look for K patches around identity
            Bounds2i line(Vec2i(0, width), Vec2i(i.y, i.y));
            Bounds2i bounds = Bounds2i(i, K) & line;
            int ok = 0;
            for(int x = bounds.min[0], X = bounds.max[0]; x <= X; ++x){
                // we insert the identity (0 disparity)
                TargetPatch q(TargetPoint(x, i.y));
                PatchData pd(q, dist(i, q));
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
                    init(i);
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

#endif	/* INT_K_DISP_H */

