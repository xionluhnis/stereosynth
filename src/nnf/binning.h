/* 
 * File:   binning.h
 * Author: Alexandre Kaspar
 *
 * Created on November 30, 2014, 9:25 PM
 */

#ifndef BINNING_H
#define	BINNING_H

#include "trypatch.h"

namespace pm {
	
	template < typename Patch = Patch2ti>
	struct PatchProvider {
		
		void query(const Point2i &i);
		Patch next();
		bool empty() const;
	};

	// TODO implement and design patch provider
	template < typename Patch = Patch2ti, typename DistValue = float, int K = 7 >
    struct Binning {
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        bool operator()(const Point2i &i, bool){
			bool success = false;
			provider.query(i);
			while(!provider.empty()){
				const Patch &p = provider.next();
				success |= kTryPatch<K, Patch, DistValue>(nnf, i, p);
			}
			return success;
		}

        Binning(NNF *n, const PatchProvider<Patch> &p) : nnf(n), provider(p) {}
		
	private:
		NNF *nnf;
		PatchProvider<Patch> provider;
    };
	
	template < typename Patch, typename DistValue >
    struct Binning<Patch, DistValue, 1> {
        typedef NearestNeighborField<Patch, DistValue, 1> NNF;

        bool operator()(const Point2i &i, bool){
			bool success = false;
			provider.query(i);
			while(!provider.empty()){
				const Patch &p = provider.next();
				success |= tryPatch<Patch, DistValue>(nnf, i, p);
			}
			return success;
		}

        Binning(NNF *n, const PatchProvider<Patch> &p) : nnf(n), provider(p) {}
		
	private:
		NNF *nnf;
		PatchProvider<Patch> provider;
    };
	
}

#endif	/* BINNING_H */

