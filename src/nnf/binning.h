/* 
 * File:   binning.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 30, 2014, 9:25 PM
 */

#ifndef BINNING_H
#define	BINNING_H

#include "trypatch.h"

namespace pm {
	
	template < typename Patch = Patch2ti, int M = 4>
	struct PatchProvider {
		
		void query(const Point2i &i);
		Patch next();
		bool empty() const;
	};

	// TODO implement and design patch provider
	template < typename Patch = Patch2ti, typename DistValue = float, int K = 7, int M = 4 >
    struct Binning {
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        uint operator()(const Point2i &i, bool){
			uint success = 0;
			provider.query(i);
			while(!provider.empty()){
				const Patch &p = provider.next();
				success += kTryPatch<K, Patch, DistValue>(nnf, i, p);
			}
			return success;
		}

        Binning(NNF *n, const PatchProvider<Patch, M> &p) : nnf(n), provider(p) {}
		
	private:
		NNF *nnf;
		PatchProvider<Patch, M> provider;
    };
	
	template < typename Patch, typename DistValue, int M = 4 >
    struct Binning<Patch, DistValue, 1, M> {
        typedef NearestNeighborField<Patch, DistValue, 1> NNF;

        uint operator()(const Point2i &i, bool){
			uint success = 0;
			provider.query(i);
			while(!provider.empty()){
				const Patch &p = provider.next();
				success += tryPatch<Patch, DistValue>(nnf, i, p);
			}
			return success;
		}

        Binning(NNF *n, const PatchProvider<Patch, M> &p) : nnf(n), provider(p) {}
		
	private:
		NNF *nnf;
		PatchProvider<Patch, M> provider;
    };
	
}

#endif	/* BINNING_H */

