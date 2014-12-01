/* 
 * File:   fwd_enrichment.h
 * Author: Alexandre Kaspar
 *
 * Created on November 30, 2014, 8:52 PM
 */

#ifndef FWD_ENRICHMENT_H
#define	FWD_ENRICHMENT_H

#include "trypatch.h"

namespace pm {

	template < typename Patch = Patch2ti, typename DistValue = float, int K1 = 7, int K2 = K1 >
    struct ForwardEnrichment {
        typedef NearestNeighborField<Patch, DistValue, K1> NNF1;
		typedef NearestNeighborField<Patch, DistValue, K2> NNF2;

        bool operator()(const Point2i &i, bool){
			bool success = false;
			
			// save the current K1 patches
			Patch patches[K1];
			for(int k1 = 0; k1 < K1; ++k1){
				patches[k1] = nnf->patch(i, k1);
			}
			
			// try all the extension patches!
			// TODO maybe try only a subset of them
			for(int k1 = 0; k1 < K1; ++k1){
				// for each K1 patches at our position
				// try all K2 extensions from the extension NNF
				for(int k2 = 0; k2 < K2; ++k2) {
					const Patch &p = extension->patch(patches[k1], k2);
					success |= kTryPatch<K1, Patch, DistValue>(nnf, i, p);
				}
			}
			return success;
		}

        ForwardEnrichment(NNF1 *n, NNF2 *e) : nnf(n), extension(e) {}
		
	private:
		NNF1 *nnf;
		NNF2 *extension; // read-only
    };
	
	template < typename Patch, typename DistValue, int K2>
    struct ForwardEnrichment<Patch, DistValue, 1, K2> {
        typedef NearestNeighborField<Patch, DistValue, 1> NNF1;
		typedef NearestNeighborField<Patch, DistValue, K2> NNF2;

        bool operator()(const Point2i &i, bool){
			bool success = false;
			Patch p = nnf->patch(i); // /!\ not the reference as it may change!!!
			
			// try all K2 extensions
			for(int k = 0; k < K2; ++k){
				success |= tryPatch<Patch, DistValue>(nnf, i, p);
			}
			return success;
		}

        ForwardEnrichment(NNF1 *n, NNF2 *e) : nnf(n), extension(e) {}
		
	private:
		NNF1 *nnf;
		NNF2 *extension;
    };
	
}


#endif	/* FWD_ENRICHMENT_H */

