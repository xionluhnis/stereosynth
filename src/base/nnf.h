/*******************************************************************************
 * nnf.h - nearest neighbor field
 *******************************************************************************
 * Add license here...
 *******************************/

#ifndef NNF_H
#define NNF_H

#include <limits>
#include <distance.h>
#include <patch.h>
#include <texture.h>
#include "ringbuffer.h"

namespace pm {

	struct CompletenessParameters {
		// raw penalty
		float penalty;
		// weighting
		float weight;
		float invWeight;
		float exponent;
		// biased search
		float absoluteThreshold; // 0 => only relative threshold
		float relativeThreshold;

		CompletenessParameters() : penalty(0.0f), weight(1.0f), invWeight(1.0f), exponent(1.0f),
		absoluteThreshold(0.0f), relativeThreshold(1.0f) {
		}

		inline float normalize(float occ) const {
			// occ in [0;1] => great
			if (occ <= 1.0f) occ *= invWeight;
				// occ in (1;+inf) => bad
			else occ *= weight;
			// occ exponentiation
			if (exponent != 1.0f) occ = std::pow(occ, exponent);
			return occ;
		}

		inline float penaltyTerm(float occDensity) const {
			return normalize(occDensity) * penalty;
		}

		operator bool() const {
			return penalty != 0.0f;
		}

		bool operator !() const {
			return penalty == 0.0f;
		}
	};
	
	struct SimilarityIndex {
		int y, x;
		SimilarityIndex(int y, int x) : y(y), x(x){}
		SimilarityIndex() : y(0), x(0){}
	};

	/**
	 * \brief Nearest neighbor field implementation for a given patch type
	 */
	template <typename Patch, typename Scalar>
	class NearestNeighborField {
	public:
		typedef Patch TargetPatch;
		typedef typename Patch::OriginalPatchType SourcePatch;
		typedef typename Distance<Patch, Scalar>::Function DistFunc;
		typedef Scalar DistValType;
		typedef float OccType;
		typedef Vec<Scalar, 3> Color;
		// Self mapping of the target to itself
		typedef NearestNeighborField<Patch, Scalar> Extension;

		const int width, height, size;
		const float sourceArea, targetArea, patchArea;
		const float bestPixelOcc, bestPatchOcc; //< best occurrence count to reach for completeness

		/**
		 * \brief Create a nnf mapping square patches in a source image
		 * to equivalent patches in a target image.
		 * 
		 * /!\ The field size cover the whole image, but border pixels are less
		 * covered by patches. This is normal.
		 * 
		 * For a source of size [H, W], the field is of size [H-P+1, W-P+1]
		 * where P is the patch size.
		 * => the last covered pixel on the right is at pos W-P+(P-1) = W-1
		 *		where W-P is the right-most element of the field,
		 *		and P-1 is the right-most relative element of any patch
		 * 
		 * Validity: if patches were single-pixels, then we cover the whole image.
		 * 
		 * \param src
		 *			the source image (input, from)
		 * \param trg
		 *			the target image (output, to)
		 * \param withDistances
		 *			whether distances are meaningful (or only patches if false)
		 * \param d
		 *			the distance function
		 * \param cp
		 *			the completeness weight
		 */
		NearestNeighborField(const Texture *src, const Texture *trg, bool withDistances = true,
				DistFunc d = NULL, CompletenessParameters cp = CompletenessParameters(), int mpd = 0) :
		width(src->cols - Patch::width() + 1), height(src->rows - Patch::width() + 1),
		size(width * height),
		sourceArea(src->rows * src->cols), targetArea(trg->rows * trg->cols), patchArea(Patch::width() * Patch::width()),
        /**
         * We pack (size * patchArea) pixels (from the source patches) into targetArea pixels
         * => best uniform occurrence count = (size * patchArea / targetArea)
         */
		bestPixelOcc(size / targetArea * patchArea),
        // best value for a patch occ without considering boundaries
        bestPatchOcc(bestPixelOcc * patchArea),
		source(src), target(trg),
		distFunc(d),
		minPatchDisp(mpd),
		patches(NULL), distances(NULL), compParams(cp),
		occurrences(NULL), similarities(NULL),
		incompleteSample(-1, -1), jumpBuffer(20){
			patches = new Patch[size];
			// we may not need distances
			if (withDistances) {
				distances = new DistValType[size];
				// default distance is infinite (not computed yet)
				std::fill(distances, distances + size, std::numeric_limits<DistValType>::max());

				// completeness map
				if (cp) {
					int osize = trg->cols * trg->rows;
					occurrences = new OccType[osize](); // 0 by default!
				}
			}
			// Note: no distance 
			// => no need to change anything
			// => no need for the rng
		}
        
        NearestNeighborField(int w, int h) : 
        width(w), height(h), size(width * height),
        sourceArea((w + Patch::width() - 1) * (h + Patch::width() - 1)),
        targetArea(0), patchArea(Patch::width() * Patch::width()),
		bestPixelOcc(0), bestPatchOcc(0), // because they are const! not to be used!
        source(NULL), target(NULL), distFunc(NULL), minPatchDisp(0),
        patches(NULL), distances(NULL), occurrences(NULL), similarities(NULL),
		incompleteSample(-1, -1), jumpBuffer(20){
            // no distance or other specific data except the patches
            patches = new Patch[size];
        }

		~NearestNeighborField() {
			if (patches != NULL) delete[] patches;
			if (distances != NULL) delete[] distances;
			if (occurrences != NULL) delete[] occurrences;
			if (similarities != NULL) delete[] similarities;
		}

		////////////////////////////////////////////////////////////////////////
		///// Random init //////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		inline void randomize() {
			using patch::randomInit;
#if _OPENMP
#pragma omp parallel for
#endif
			for (int i = 0; i < size; ++i) {
				int x = i % width;
				int y = i / width;
				// random init
				int r = 0;
				do {
					randomInit(pm::unif01, target, patches[i]);
					if(minPatchDisp <= 0) break;
					PatchDisplacement d = std::abs(patches[i].y - y) + std::abs(patches[i].x - x);
					if(d >= minPatchDisp) break;
				} while(++r < 100);
				// patches[i].reset(Patch::random(unif01, target));

				// update distance
				distance(y, x) = distance(y, x, patches[i]);

				// update occurrences
				if (withOccurrences()) {
					occurrence(patches[i], 1.0f);
				}
			}
		}	
		inline void scramble(const Image &mask) {
			using patch::randomInit;
#if _OPENMP
#pragma omp parallel for collapse(2)
#endif
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					// only for negative mask values
					if(mask.at<float>(y, x) >= 0.0f) continue;
				
					// random new patch
					Patch newPatch;
					randomInit(pm::unif01, target, newPatch);
					
					set(y, x, newPatch);
				}
			}
		}
		bool check() {
			bool ok = true;
			for (int i = 0; i < size; ++i) {
				const Patch &p = patches[i];
				int x = i % width;
				int y = i / width;
				using patch::isWithin;
				if (!isWithin(target, p)) {
					float d[10];
					float *data = &d[0];
					p.store(data, Patch::dimensions());
					std::cout << "p@" << y << "/" << x << " = ";
					for (int i = 0; i < Patch::dimensions(); ++i) std::cout << d[i] << ", ";
					std::cout << "| not within (" << target->rows << " / " << target->cols << ")\n";
					ok = false;
				}
			}
			return ok;
		}

		////////////////////////////////////////////////////////////////////////
		///// Distances ////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		inline DistValType distance(int y, int x) const {
			return distances[y * width + x];
		}
		inline DistValType &distance(int y, int x) {
			return distances[y * width + x];
		}
		/// Compute the distance between the current patch and a new one (up to a best value after which we do not process)
		inline DistValType distance(int y, int x, const Patch &trg, DistValType bestDist = -1.0f) const {
			const SourcePatch srcPatch(y, x);
			DistValType d = distFunc(source, target, srcPatch, trg, bestDist);
			if (!std::isfinite(d) || (d + 1) == d) {
				std::cout << "Invalid distance: d=" << d << " for p@" << y << "/" << x << ", bestDist=" << bestDist << "\n";
			}
			return d;
		}
		inline void calcDistances() {
#if _OPENMP
#pragma omp parallel for collapse(2)
#endif
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					// recompute the distance
					distance(y, x) = distance(y, x, get(y, x));
				}
			}
		}
		DistValType maxDistance() const {
			DistValType maxDist = -1.0;
			for (int i = 0; i < size; ++i) {
				DistValType d = distances[i];
				if (d > maxDist) maxDist = d;
			}
			return maxDist;
		}
		DistValType meanDistance() const {
			long double sum = 0;
			for (int i = 0; i < size; ++i) {
				sum += distances[i];
			}
			return DistValType(sum / size);
		}

		////////////////////////////////////////////////////////////////////////
		///// Occurrence map ///////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		/// Return the current pixel occurrence count
		inline OccType occurrence(int y, int x) const {
			if (y >= target->rows || x >= target->cols || x < 0 || y < 0) {
				std::cerr << "Occurrence error! @" << y << "/" << x << " on " << target->rows << "/" << target->cols << "\n";
				return 0.0f;
			}
			return occurrences[y * target->cols + x];
		}
		OccType dummy;
		inline OccType &occurrence(int y, int x) {
			if (y >= target->rows || x >= target->cols || x < 0 || y < 0) {
				std::cerr << "Occurrence error! @" << y << "/" << x << " on " << target->rows << "/" << target->cols << "\n";
				dummy = 0.0f;
				return dummy;
			}
			return occurrences[y * target->cols + x];
		}
		inline float occurrence(const Patch &p) const {
			float sum = 0.0f;
			for (int py = 0, n = Patch::width(); py < n; ++py) {
				for (int px = 0; px < n; ++px) {
					Point<int> l = p.transform(py, px).within(target);
					// XXX what about not only nearest neighbor here?
					sum += occurrence(l.y, l.x);
				}
			}
			return sum;
		}
        inline OccType occDensity(int y, int x) const {
            OccType targetOcc = bestPixelOcc * expectedOverlap(y, x);
            return occurrence(get(y, x)) / targetOcc;
        }
        inline OccType meanPatchOccRatio() const {
            if(!withOccurrences()) return 0;
            OccType patchOcc = 0;
            for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
                    patchOcc += occurrence(get(y, x));
				}
			}
            patchOcc /= size;
            return patchOcc / bestPatchOcc;
        }
		/// Compute per pixel occurrence counts from a patch and add it to the map
		inline void occurrence(const Patch &p, float f) {
			for (int py = 0, n = Patch::width(); py < n; ++py) {
				for (int px = 0; px < n; ++px) {
					Point<int> l = p.transform(py, px).within(target);
					// XXX what about not only nearest neighbor here?
					OccType &occ = occurrence(l.y, l.x);
					occ = std::max(0.0f, occ + f);
				}
			}
		}
		inline void calcCompleteness() {
			if (!compParams) return;
			// completeness map
			if (occurrences == NULL) {
				int osize = target->cols * target->rows;
				occurrences = new OccType[osize](); // 0 by default!
			}
			// each patch generates pixel occurences
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					occurrence(get(y, x), 1.0f);
				}
			}
		}
		inline bool withOccurrences() const {
			return occurrences != NULL;
		}
		// checking absolute and relative threshold for a new potential patch
		inline bool checkCandidateCompleteness(int y, int x, const Patch &newPatch,
			OccType &curOcc = -1, OccType &newOcc = -1) const {
			if(!withOccurrences()) {
				return true;
			}
			const Patch &curPatch = get(y, x);
			
			if(curOcc < 0) curOcc = occurrence(curPatch);
			if(newOcc < 0){
				OccType overlap = 0; // TODO remove the overlapping area between cur and new
				newOcc = occurrence(newPatch) + patchArea - overlap; // new occurrences that would get introduced!
			}
			// absolute threshold
			if (newOcc <= bestPixelOcc * expectedOverlap(y, x) * compParams.absoluteThreshold)
				return true;
			else // else we look at whether we're better than the previous one
				return newOcc <= curOcc * compParams.relativeThreshold;
		}
		
		////////////////////////////////////////////////////////////////////////
		///// Extension NNF ////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
	public:
		inline void useExtension(const Extension *extNNF) {
			if(extNNF == NULL) return;
			if(similarities == NULL) { // /!\ withSimilarities is based on it being != NULL
				similarities = new SimilarityIndex[size];
				for(int y = 0; y < height; ++y) {
					for(int x = 0; x < width; ++x) {
						similarity(y, x) = basicSimilarity(y, x);
					}
				}
			}
			if(extNNF->targetArea != extNNF->sourceArea
					|| extNNF->targetArea != targetArea
					|| extNNF->source->width != target->width
					|| extNNF->source->height != target->height) {
				std::cerr << "Invalid extension domain: WxH = " << extNNF->source->width;
				std::cerr << "x" << extNNF->source->height << " instead of " << target->width;
				std::cerr << "x" << target->height << " (source: " << source->width;
				std::cerr << "x" << source->height << ")\n";
			}
			extension = extNNF;
		}
		inline const SimilarityIndex *currentSimilarity(int y, int x) const {
			return &similarities[y * width + x];
		}
		inline const SimilarityIndex *nextSimilarity(int y, int x) {
			SimilarityIndex &si = similarities[y * width + x];
			// TODO apply some transformation for affine patches?
			const Patch &p = extension->get(si.y, si.x);
			// set similarity position
			si.y = p.y;
			si.x = p.x;
			// return the pointer to it
			return &si;
		}
		inline SimilarityIndex basicSimilarity(int y, int x) const {
			const Patch &p = get(y, x);
			return SimilarityIndex(p.y, p.x);
		}
		inline bool withSimilarities() const {
			return similarities != NULL;
		}
	protected:
		inline SimilarityIndex &similarity(int y, int x) {
			return similarities[y * width + x];
		}
		
		////////////////////////////////////////////////////////////////////////
		///// Coherence ////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
	public:
		inline typename Patch::Coherence coherence(const Patch& p, int y, int x) const {
			using patch::coherence;
			typename Patch::Coherence coh = 0;
			if (x > 0) {
				// left coherence
				coh += coherence(p, get(y, x - 1), 0, -1);
				// right coherence
				if (x < width - 1) coh += coherence(p, get(y, x + 1), 0, 1);
			} else {
				// right coherence
				coh += coherence(p, get(y, x + 1), 0, 1);
			}
			if (y > 0) {
				// top coherence
				coh += coherence(p, get(y - 1, x), -1, 0);
				// bottom coherence
				if (y < height - 1) coh += coherence(p, get(y + 1, x), 1, 0);
			} else {
				coh += coherence(p, get(y + 1, x), 1, 0);
			}
			return coh;
		}
		inline typename Patch::Coherence coherence(int y, int x) const {
			return coherence(get(y, x), y, x);
		}
		inline typename Patch::Coherence coherence(const SimilarityIndex *si) const {
			return coherence(si->y, si->x);
		}
		inline typename Patch::Coherence coherence() const {
			typename Patch::Coherence sum = 0;
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					sum += coherence(get(y, x), y, x);
				}
			}
			return sum;
		}

		////////////////////////////////////////////////////////////////////////
		///// Patches //////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		inline const Patch& get(int y, int x) const {
			return patches[y * width + x];
		}
		inline const Patch& get(const SimilarityIndex *si) const {
			return get(si->y, si->x);
		}
		inline Patch &get(int y, int x) {
			return patches[y * width + x];
		}
		/// Update a patch
		inline void set(int y, int x, const Patch &newPatch, DistValType newDist = -1.0) {
			// possibly update occurrences
			if (withOccurrences()) {
				const Patch &oldPatch = get(y, x);
				occurrence(oldPatch, -1.0f);
				occurrence(newPatch, +1.0f);
			}

			// change patch
			get(y, x) = newPatch; // using copy constructor and assignment operator
			// update the distance
			if (newDist < 0) {
				newDist = distance(y, x, newPatch, std::numeric_limits<DistValType>().max());
			}
			distance(y, x) = newDist;
			
			// possibly reset the similarity index
			if (withSimilarities()) {
				similarity(y, x) = basicSimilarity(y, x);
			}
		}
		inline OccType expectedOverlap(int y, int x) const {
			// int margin = Patch::width();
			// int overlap = (margin - std::max(margin - x, 0)) * (margin - std::max(margin - y, 0)); // TODO correct it!
			// return overlap;
			return patchArea; // simplest approximation
		}
		/// Try a new patch, and update the nnf in case it's better
		inline bool tryPatch(int y, int x, const Patch &newPatch, float curOcc = -1.0f, float newOcc = -1.0f) {
			// require minimum displacement from identity
			if(minPatchDisp > 0) {
				PatchDisplacement d = std::abs(y - newPatch.y) + std::abs(x - newPatch.x); // L1 distance
				if(d < minPatchDisp) return false;
			}
			
			const Patch &curPatch = get(y, x);
			// no need to try if they're the same
			if(curPatch == newPatch) {
				return false;
			}
			// distances
			DistValType curDist = distance(y, x);
			DistValType newDist = distance(y, x, newPatch, curDist);

			// occurrences
			if (withOccurrences()) {
				// recompute only if needed!
				if (curOcc == -1.0f) curOcc = occurrence(curPatch);
				if (newOcc == -1.0f) {
					OccType overlap = 0; // TODO remove the overlapping area between cur and new
					newOcc = occurrence(newPatch) + patchArea - overlap; // new occurrences that would get introduced!
				}
			} else curOcc = newOcc = 0.0f;

			// completeness penalty
			DistValType curRes = curDist;
			DistValType newRes = newDist;
			OccType targetOcc = bestPixelOcc;
			if (curOcc != newOcc) {
				OccType overlap = expectedOverlap(y, x);
				targetOcc *= overlap;
				curRes += compParams.penaltyTerm(curOcc / targetOcc);
				newRes += compParams.penaltyTerm(newOcc / targetOcc);
			}

			// if the result is better, we update the patch
			if (newRes < curRes) {
				/*std::cout << "[+] " << newRes << " < " << curRes;
				std::cout << ", comp: " << newOcc / targetOcc << " / " << curOcc / targetOcc;
				std::cout << " (best=" << targetOcc << ") at (";
				std::cout << y << " / " << x << ")\n"; */
				set(y, x, newPatch, newDist);
				return true;
			}
			// no change
			return false;
		}
		
		////////////////////////////////////////////////////////////////////////
		///// Propagation //////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		inline bool propagation(int y, int x, int dy, int dx, bool checkOcc = false) {
			// delta patch
			Patch delta;
			
			// ADL
			using patch::deltaPatch;
			using patch::isWithin;
			
			// previous location
			int py = y - dy;
			int px = x - dx;
			
			// flags
			bool didProp = false;
			
			// dy patch
			if (py >= 0 && py < height) {
				deltaPatch(get(py, x), delta, dy, 0);
				if (isWithin(target, delta)){
					if(checkOcc){
						OccType curOcc = -1, newOcc = -1;
						if(checkCandidateCompleteness(y, x, delta, curOcc, newOcc)){
							tryPatch(y, x, delta, curOcc, newOcc) && (didProp = true);
						}
					} else {
						tryPatch(y, x, delta) && (didProp = true);
					}
				}
			}
			
			// dx patch
			if (px >= 0 && px < width) {
				deltaPatch(get(y, px), delta, 0, dx);
				if (isWithin(target, delta)){
					if(checkOcc){
						OccType curOcc = -1, newOcc = -1;
						if(checkCandidateCompleteness(y, x, delta, curOcc, newOcc)){
							tryPatch(y, x, delta, curOcc, newOcc) && (didProp = true);
						}
					} else {
						tryPatch(y, x, delta) && (didProp = true);
					}
				}
			}
			
			return didProp;
		}
		inline bool similarityPropagation(int y, int x, int dy, int dx, bool checkOcc = false) {
			
			Patch delta;
			
			// ADL
			using patch::deltaPatch;
			using patch::isWithin;
			
			// previous location
			int py = y - dy;
			int px = x - dx;
			
			// flags
			bool didSim = false;
			
			// current similarity index
			const SimilarityIndex *si = currentSimilarity(y, x);
			if(checkOcc){
				OccType curOcc = -1, newOcc = -1;
				const Patch &simPatch = extension->get(si);
				if(checkCandidateCompleteness(y, x, simPatch, curOcc, newOcc)){
					tryPatch(y, x, simPatch, curOcc, newOcc) && (didSim = true);
				}
			} else {
				didSim = tryPatch(y, x, extension->get(si));
			}

			// check whether propagation makes sense
			if (extension->coherence(si) < 4.0f) {
				int qy = si->y - dy;
				// dy patch
				if (qy >= 0 && qy < extension->height) {
					deltaPatch(extension->get(qy, si->x), delta, dy, 0);
					if (isWithin(target, delta)){
						if(checkOcc){
							OccType curOcc = -1, newOcc = -1;
							if(checkCandidateCompleteness(y, x, delta, curOcc, newOcc)){
								tryPatch(y, x, delta, curOcc, newOcc) && (didSim = true);
							}
						} else {
							tryPatch(y, x, delta) && (didSim = true);
						}
					}
				}
				int qx = si->x - dx; // si may point to a new index now!
				// dx patch
				if (qx >= 0 && qx < extension->width) {
					deltaPatch(extension->get(si->y, qx), delta, 0, dx);
					if (isWithin(target, delta)){
						if(checkOcc){
							OccType curOcc = -1, newOcc = -1;
							if(checkCandidateCompleteness(y, x, delta, curOcc, newOcc)){
								tryPatch(y, x, delta, curOcc, newOcc) && (didSim = true);
							}
						} else {
							tryPatch(y, x, delta) && (didSim = true);
						}
					}
				}
			}
			
			return didSim;
		}
        
        ////////////////////////////////////////////////////////////////////////
		///// Random Search ////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		/// Do random search
		inline bool randomSearch(int y, int x, int maxWindowSize, int maxRS = 1) {
			using pm::random;
			// the patch to go against
			const Patch &curPatch = get(y, x);
			// generate a random patch
			Patch newPatch;
			bool valid = false;
			float curOcc = withOccurrences() ? occurrence(curPatch) : -1.0f;
			float newOcc = -1.0f;
			while (maxRS-- > 0) {
				valid = random(pm::unif01, target, curPatch, newPatch, maxWindowSize, minPatchDisp, y, x);
				if (valid) {
					if(checkCandidateCompleteness(y, x, newPatch, curOcc, newOcc)){
						break;
					} else {
						newOcc = -1.0f; // reset for the next check
					}
				}
			}
			if (valid) {
				// replace if distance is better
				bool res = tryPatch(y, x, newPatch, curOcc, newOcc);
				if(!res && withOccurrences()) {
					storeJumpSample(Point2i(newPatch.x, newPatch.y));
				}
				return res;
			}
			// no change
			return false;
		}
        /// Do aligned search
        inline bool alignedSearch(int y, int x, const Point2f &g1, const Point2f &g2, float jitter, int maxRS = 1) {
            using pm::aligned;
			// the patch to go against
			const Patch &curPatch = get(y, x);
			// generate a random patch
			Patch newPatch;
			bool valid = false;
			float curOcc = withOccurrences() ? occurrence(curPatch) : -1.0f;
			float newOcc = -1.0f;
			while (maxRS-- > 0) {
				valid = aligned(pm::unif01, target, curPatch, newPatch, g1, g2, jitter, minPatchDisp, y, x);
				if (valid && withOccurrences()) {
					// valid only if it increases completeness
					newOcc = occurrence(newPatch) + patchArea;
					// absolute threshold
					if (newOcc <= bestPixelOcc * expectedOverlap(y, x) * compParams.absoluteThreshold)
						valid = true;
					else // else we look at whether we're better than the previous one
						valid = newOcc <= curOcc * compParams.relativeThreshold;
				}
				// if the new patch is valid, no need to search more
				if (valid) break;
			}
			if (valid) {
				// replace if distance is better
				bool res = tryPatch(y, x, newPatch, curOcc, newOcc);
				if(!res && withOccurrences()) {
					storeJumpSample(Point2i(newPatch.x, newPatch.y));
				}
				return res;
			}
			// no change
			return false;
        }
		// Do incomplete search
		inline bool incompleteSearch(int y, int x, int maxRS = 1) {
			Point2i sample;
			while(maxRS-- > 0){
				if(pickIncompleteSample(sample)){
					// use it as new patch (with low completeness)
					Patch lowCompPatch;
					lowCompPatch.x = sample.x;
					lowCompPatch.y = sample.y;
					return tryPatch(y, x, lowCompPatch);
				}
			}
			// no change
			return false;
		}
		
		////////////////////////////////////////////////////////////////////////
		///// Incomplete Sampling //////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		
		inline float incompleteness(int y, int x) const {
			OccType targetOcc = occurrence(y, x); // pixel wise, not patch wise
			if(targetOcc >= bestPixelOcc) return 0.0f;
			else return 1.0f - (targetOcc / bestPixelOcc);
		}
		inline void getSampleCount(int &validCount, int &sampleCount, 
			int &jumpCount, int &resetCount) const {
			validCount = validSampleCount;
			sampleCount = totalSampleCount;
			jumpCount = sampleJumpCount;
			resetCount = sampleResetCount;
		}
		inline void updateJumpBuffer() {
			// /!\ Attention, we're working in the target space
			int H = target->height - Patch::width(); // -1 for the 0-based indexing, +1 for the first pixel of the last patch
			int W = target->width - Patch::width();
			int size = H * W;
			// random traversal
			std::vector<int> indices(size, 0);
			for(int i = 0; i < size; ++i) indices[i] = i;
			for(int p = 0, sample = 0; p < size; ++p) {
				// online shuffle to get the current index
				int j = uniform(unif01, p, size - 1);
				if(p != j) std::swap(indices[p], indices[j]); // needed to traverse all elements
				int i = indices[p]; // the current element
				// the corresponding position
				int x = i % W;
				int y = i / W;
				float f = incompleteness(y, x);
				if(f > 0) {
					storeJumpSample(Point2i(x, y));
					++sample;
					if(sample >= jumpBuffer.size()){
						return;
					}
				}
			}
			// reset counts
			validSampleCount = totalSampleCount = sampleJumpCount = sampleResetCount = 0;
		}
		inline void initJumpBuffer(int size, float sigmaFactor) {
			validSampleCount = totalSampleCount = sampleJumpCount = sampleResetCount = 0;
			if(size > 0 && jumpBuffer.capacity() != size){
				jumpBuffer = RingBuffer<Point2i>(size);
			}
			jumpSigma = sigmaFactor * (std::min(target->width, target->height) - Patch::width());
			// we should update the buffer now
			updateJumpBuffer();
		}
		
	protected:
		inline bool validStartSample(Point2i &sample, float &f = 0.0f) {
			if(incompleteSample.x >= 0){
				f = incompleteness(incompleteSample.y, incompleteSample.x);
				if(f > 0) {
					sample = incompleteSample;
					return true;
				}
			}
			++sampleResetCount;
			
			// we need to replace our current sample
			// 1. try in the jump buffer
			while(!jumpBuffer.empty()){
				sample = jumpBuffer.pop(); // or should we just shift?
				f = incompleteness(sample.y, sample.x);
				if(f > 0){
					incompleteSample = sample; // store it!
					// should we push it at the end of the buffer?
					return true;
				}
			}
			
			// 2. try randomly (this shouldn't happen often!)
			// /!\ in target space
			sample = Point2i(
				uniform(unif01, 0, target->width - Patch::width()), 
				uniform(unif01, 0, target->height - Patch::width())
			);
			f = incompleteness(sample.y, sample.x);
			if(f <= 0) --sampleResetCount;
			return f > 0;
		}
		inline void storeJumpSample(const Point2i &sample) {
			float f_new = incompleteness(sample.y, sample.x);
			if(f_new > 0) {
				if(jumpBuffer.empty()){
					jumpBuffer.push(sample);
				} else {
					// maybe replace current pointed element
					Point2i &cur = jumpBuffer.shift();
					float f_old = incompleteness(cur.y, cur.x);
					if(f_old <= 0.0f) {
						// the pointed element is not valid anymore, we replace it
						cur = sample;
					} else if (!jumpBuffer.full()){
						// we add ourself to the end of the buffer since there's space
						jumpBuffer.push(sample);
					} else if(f_new > f_old) {
						// we replace the current pointed element
						// since, while there's no space, the new is better
						cur = sample;
					}
					// else we throw that sample in the void
					// ...
				}
			}
		}
		inline bool pickIncompleteSample(Point2i &sample) {
			// do we have a sample to start with?
			float f_prev;
			++totalSampleCount;
			if(validStartSample(sample, f_prev)){
				++validSampleCount;
				// Note: sample is already set, we are trying to pick the next one
				
				// Metropolis algorithm
				// @see http://web.mit.edu/~wingated/www/introductions/mcmc-gibbs-intro.pdf
				// @see http://en.wikipedia.org/wiki/Metropolis%E2%80%93Hastings_algorithm
				// 1 = pick a new point from sample
				// /!\ in target space, workspace = size != not last element!
				Point2i workspace(target->width - Patch::width() + 1, target->height - Patch::width() + 1);
				float angle = unif01() * M_PI * 2;
				float radius = std::abs(jumpSigma * 0.5 + gaussian(unif01, jumpSigma));
				// std::cout << "r: " << radius << " ";
				// ring-shaped distribution using a radius that is the abs(gaussian + shift)
				Point2i newSample = sample + Point2i(
					round(radius * std::cos(angle)),
					round(radius * std::sin(angle))
				);
				// put it back to the workspace (easy for positive values, harder for negative values)
				if(newSample.x >= workspace.x){
					newSample.x = newSample.x % workspace.x;
				}else if(newSample.x < 0) {
					// it may span more than one time the space => not just + W % W
					newSample.x = workspace.x - 1 - ((-newSample.x) % workspace.x);
				}
				if(newSample.y >= workspace.y){
					newSample.y = newSample.y % workspace.y;
				}else if(newSample.y < 0) {
					newSample.y = workspace.y - 1 - ((-newSample.y) % workspace.y);
				}
				
				// is it a valid jump after quantization?
				if(newSample.x == incompleteSample.x && newSample.y == incompleteSample.y) {
					return true; // not a jump though
				}
				
				// get the occurrence data
				float f_new = incompleteness(newSample.y, newSample.x);
				
				// 2 = conditional move with prob a = max(1, f_new / f_prev)
				++sampleJumpCount;
				if(f_new >= f_prev) {
					// a >= 1
					// => we move
					incompleteSample = newSample;
				} else if(unif01() * f_prev <= f_new) {
					// rand() <= a
					// => we move, which happens with prob a
					incompleteSample = newSample;
				} else {
					--sampleJumpCount;
				}
				// else no move, happening with prob 1-a
				return true;
			} else {
				return false;
			}
		}
		
	public:

		// Source and target domains
		const Texture *source, *target;

		// Completeness weight
		CompletenessParameters compParams;
		
		// Distance function
		DistFunc distFunc;
		// Displacement from identity (self-nnf)
		int minPatchDisp;

	protected:

		// Row-major y, Column-minor x
		Patch *patches;
		DistValType *distances;
		OccType *occurrences;
		// Augmented NNF
		SimilarityIndex *similarities;
		const Extension *extension;
		// Incomplete sampling
		Point2i incompleteSample;
		RingBuffer<Point2i> jumpBuffer;
		float jumpSigma;
		int validSampleCount, totalSampleCount, sampleJumpCount, sampleResetCount;
	};
	
}

#endif