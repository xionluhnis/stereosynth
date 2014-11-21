/*******************************************************************************
 * basic.h - basic patch implementation
 *******************************************************************************
 * Add license here...
 *******************************/

#ifndef BASIC_H
#define	BASIC_H

#include "../patch.h"

namespace pm {

	template <typename T, int PatchWidth = DEFAULT_PATCH_SIZE >
	struct BasicPatch : public BasicGrid< BasicPatch<T, PatchWidth> > {
		/// the width that can be inferred statically
		static const int staticSize = PatchWidth;
		
		/// Location type
		typedef T Scalar;
		/// Pixel location within a texture
		typedef Point<T> PixLoc;
		typedef Point<int> Index;
		/// Patch types
		typedef BasicPatch<int, PatchWidth> OriginalPatchType;
		typedef T Coherence;
		
		// template definitions
		inline static int width(int newSize = 0) {
			if (newSize != 0) throw "Static patches cannot change their size!";
			return PatchWidth;
		}

		inline static float radius() {
			return 0.5f * width;
		}

		inline static int dimensions() {
			return 2;
		}

		// the real data is here
		T x, y;

		// Simple constructor

		BasicPatch() : x(0), y(0) {
		}

		BasicPatch(T y0, T x0) : x(x0), y(y0) {
		}
		
		bool operator==(const BasicPatch<T, PatchWidth> &p) const {
			return x == p.x && y == p.y;
		}

		// points of the patch

		inline void points(PixLoc pts[4]) const {
			pts[0] = PixLoc(x, y);
			pts[1] = PixLoc(x, Scalar(y + width() - 1));
			pts[2] = PixLoc(Scalar(x + width() - 1), Scalar(y + width() - 1));
			pts[3] = PixLoc(Scalar(x + width() - 1), Scalar(y));
		}
		// center of the patch

		inline int cx() const {
			return x + int(radius());
		}

		inline int cy() const {
			return y + int(radius());
		}

		template <typename Storage>
				inline void store(Storage &out, int channels) const {
			switch (channels) {
				case 2:
					out[1] = x;
					out[0] = y;
					break;
				default:
					std::cerr << "Unknown storage of " << channels;
					std::cerr << " channels!\n";
			}
		}

		inline void load(const float *source, int channels, int offset) {
			y = source[0];
			x = source[offset];
		}
		
		inline PixLoc transform(int py, int px) const {
			return PixLoc(x + px, y + py);
		}
		inline PixLoc transform(const Index &i) const {
			return PixLoc(x + i.x, y + i.y);
		}
		inline PixLoc operator *(const Index &i) const {
			return transform(i);
		}
	};
	template <>
	inline int BasicPatch<int, 0>::width(int newSize) {
		static int size = DEFAULT_PATCH_SIZE;
		if (newSize > 0) size = newSize;
		return size;
	}
	
	template <>
	inline int BasicPatch<float, 0>::width(int newSize) {
		return BasicPatch<int, 0>::width(newSize); // delegate
	}

	/**
	 * \brief Dynamic sized basic patch
	 */
	typedef BasicPatch<int, 0> BasicPatchX;
	typedef BasicPatch<float, 0> BasicFloatPatchX;

	template <typename T, int PatchWidth>
	inline void randomInit(RNG rand, const Image *parent,
			BasicPatch<T, PatchWidth> &patch) {
		T maxX = parent->cols - BasicPatch<T, PatchWidth>::width() - 1;
		T maxY = parent->rows - BasicPatch<T, PatchWidth>::width() - 1;
		patch.x = uniform(rand, T(0), maxX);
		patch.y = uniform(rand, T(0), maxY);
	}

	template <typename T, int PatchWidth>
	inline bool random(RNG rand, const Image *parent,
			const BasicPatch<T, PatchWidth> &oldPatch,
			BasicPatch<T, PatchWidth> &newPatch,
			int windowSize) {
		typedef BasicPatch<T, PatchWidth> Patch;
		newPatch.x = uniform(rand,
				std::max(T(0), oldPatch.x - T(windowSize)),
				std::min(T(parent->cols - Patch::width()), oldPatch.x + T(windowSize)) // Not cols - P - 1!
				);
		newPatch.y = uniform(rand,
				std::max(T(0), oldPatch.y - T(windowSize)),
				std::min(T(parent->rows - Patch::width()), oldPatch.y + T(windowSize))
				);
		return true;
	}
    
    template <typename T, int PatchWidth>
	inline bool aligned(RNG rand, const Image *parent,
			const BasicPatch<T, PatchWidth> &oldPatch,
            BasicPatch<T, PatchWidth> &newPatch, 
            const Point2f &g1, const Point2f &g2, float jitter) {
        typedef BasicPatch<T, PatchWidth> Patch;
		// steps and direction type
		const bool sameDir = g1.x * g1.y >= 0;
		const Point2f step = Point2f::max(g1.abs(), Point2f(1, 1));
		
		// what are the possible steps?
        int xPosSteps = std::floor((parent->rows - Patch::width() - oldPatch.x) / step.x);
		int xNegSteps = std::floor((oldPatch.x) / step.x);
		int yPosSteps = std::floor((parent->cols - Patch::width() - oldPatch.y) / step.y);
		int yNegSteps = std::floor((oldPatch.y) / step.y);
		int posSteps, negSteps;
		if(sameDir){
			// same direction in x and y
			posSteps = std::min(xPosSteps, yPosSteps);
			negSteps = std::min(xNegSteps, yNegSteps);
		} else {
			// inverse directions, we choose arbitrarily the association
			posSteps = std::min(xPosSteps, yNegSteps);
			negSteps = std::min(xNegSteps, yPosSteps);
		}
		
		// can we go anywhere?
		if(posSteps + negSteps < 1){
			// no transformation
			newPatch.x = oldPatch.x;
			newPatch.y = oldPatch.y;
			// we failed
			return false;
		}
		
		// get our step
		int stepID = uniform(rand, 0, posSteps + negSteps);
		// direction
		int dir;
		if(stepID > posSteps){
			stepID = 1 + stepID - posSteps;
			dir = -1;
		} else if(posSteps) {
			dir = 1;
		}
		
		// compute the shift
		Point2f shift;
		if(sameDir){
			shift = (g1.x > 0 || g1.y > 0 ? g1 : -g1) * (dir * stepID);
		} else if(g1.x > 0) { // => g1.y < 0
			shift = g1 * (dir * stepID);
		} else { // => g1.y > 0
			shift = -g1 * (dir * stepID); 
			// reversed because of the definition of posSteps and negSteps
			// i.e. taken for g1.x positive
		}
		
		// is it the end or do we have to go for g2?
		if(g2.isOrigin()) {
			if(jitter == 0.0f) {
				newPatch.x = oldPatch.x + roundOrNot<T>(shift.x);
				newPatch.y = oldPatch.y + roundOrNot<T>(shift.y);
			} else {
				// add the jitter from a gaussian
				T dx = gaussian(rand, jitter); // /!\ dx and dy may be correlated, but that's fine
				T dy = gaussian(rand, jitter);
				Patch jittered(
					roundOrNot<T>(oldPatch.x + shift.x + dx),
					roundOrNot<T>(oldPatch.x + shift.y + dy)
				);
				if(isWithin(parent, jittered)){
					newPatch = jittered;
				}
			}
		} else {
			// we go for g2, recursively of course ...
			Patch tmpPatch(oldPatch.x + shift.x, oldPatch.y + shift.y);
			aligned(rand, parent, tmpPatch, newPatch, g2, Point2f(), jitter);
		}
		return isWithin(parent, newPatch);
    }

	template <typename T, int PatchWidth>
	inline void deltaPatch(
			const BasicPatch<T, PatchWidth> &patch,
			BasicPatch<T, PatchWidth> &delta,
			int dy, int dx) {
		delta.x = patch.x + dx;
		delta.y = patch.y + dy;
	}

	template <typename T, int PatchWidth>
	inline bool isWithin(const Image *parent, const BasicPatch<T, PatchWidth> &patch) {
		int w = BasicPatch<T, PatchWidth>::width();
		int maxY = parent->rows, maxX = parent->cols;
		return patch.x >= 0 && patch.y >= 0 && patch.x <= T(maxX - w) && patch.y <= T(maxY - w);
	}

	template <int PatchWidth>
	inline typename BasicPatch<int, PatchWidth>::Coherence coherence(
			const BasicPatch<int, PatchWidth> &p1, 
			const BasicPatch<int, PatchWidth> &p2, 
			int dy, int dx){
		return  p1.y + dy == p2.y && p1.x + dx == p2.x ? 1 : 0;
	}
	template <int PatchWidth>
	inline typename BasicPatch<float, PatchWidth>::Coherence coherence(
			const BasicPatch<float, PatchWidth> &p1, 
			const BasicPatch<float, PatchWidth> &p2, 
			int dy, int dx){
		return p1.y + dy == p2.y && p1.x + dx == p2.x ? 1.0f : 0.0f;
		// return  std::max(0.0f, 1.0f - std::abs(p1.y + dy - p2.y) - std::abs(p1.x + dx - p2.x));
	}
}

#endif	/* BASIC_H */

