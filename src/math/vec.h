/* 
 * File:   vec.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 15, 2014, 9:21 PM
 */

#ifndef VEC_H
#define	VEC_H

#include "defs.h"

namespace pm {
	/**
	 * \brief Small vector that stays on the stack
	 */
	template<typename T, int cn>
	struct Vec {
		typedef T scalar;
		typedef Vec<T, cn> vec;

		enum {
			typeDepth = DataDepth<scalar>::value,
			channels = cn,
			type = IM_MAKETYPE(typeDepth, channels)
		};

		//! default constructor

		Vec() {
			// note: we do not initialize data here!
		}

		Vec(T v0) {
			data[0] = v0;
		}

		Vec(T v0, T v1) {
			data[0] = v0;
			data[1] = v1;
		}

		Vec(T v0, T v1, T v2) {
			data[0] = v0;
			data[1] = v1;
			data[2] = v2;
		}

		explicit Vec(const T* vals) {
			for (int i = 0; i < channels; ++i) data[i] = vals[i];
		}

		static vec all(T alpha) {
			vec m;
			std::fill(m.data, m.data + channels, alpha);
			return m;
		}

		inline static vec zeros() {
			return all(0);
		}

		inline static vec ones() {
			return all(1);
		}

		//! convertion to another data type

		template<typename T2> operator Vec<T2, cn>() const {
			Vec<T2, cn> v2;
			for (int i = 0; i < channels; ++i) v2[i] = data[i];
			return v2;
		}

		//! dot product computed with the default precision

		inline T dot(const vec& v) const {
			T sum = 0;
			for (int i = 0; i < channels; ++i) sum += v[i] * data[i];
			return sum;
		}

		//! multiply two vectors element-wise

		inline vec mul(const vec& a) const {
			vec v;
			for (int i = 0; i < channels; ++i) v[i] = a[i] * data[i];
			return v;
		}

		//! binary operators

		inline vec operator +(const vec& a) const {
			vec v;
			for (int i = 0; i < channels; ++i) v[i] = data[i] + a[i];
			return v;
		}

		inline vec& operator +=(const vec& a) {
			for (int i = 0; i < channels; ++i) data[i] += a[i];
			return *this;
		}

		inline vec operator -(const vec& a) const {
			vec v;
			for (int i = 0; i < channels; ++i) v[i] = data[i] - a[i];
			return v;
		}

		inline vec& operator -=(const vec& a) {
			for (int i = 0; i < channels; ++i) data[i] -= a[i];
			return *this;
		}

		inline vec operator *(T x) const {
			vec v;
			for (int i = 0; i < channels; ++i) v[i] = data[i] * x;
			return v;
		}

		inline vec& operator *=(T x) {
			for (int i = 0; i < channels; ++i) data[i] *= x;
			return *this;
		}

		//! unary operator

		inline vec operator -() const {
			vec v;
			for (int i = 0; i < channels; ++i) v[i] = -data[i];
			return v;
		}
		
		//! equality operators
		inline bool operator ==(const vec &v) const {
			bool eq = true;
			for(int i = 0; eq && i < channels; ++i){
				eq &= v[i] == data[i];
			}
			return eq;
		}
		inline bool operator !=(const vec &v) const {
			return !(*this == v);
		}

		//! element access
		inline const T& operator [](int i) const {
			return data[i];
		}
		inline T& operator [](int i) {
			return data[i];
		}

		T data[channels];
	};
	
	typedef Vec<int, 2> Vec2i;
	typedef Vec<int, 3> Vec3i;
	typedef Vec<float, 2> Vec2f;
	typedef Vec<float, 3> Vec3f;
}

#endif	/* VEC_H */

