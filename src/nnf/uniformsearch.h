/* 
 * File:   uniformsearch.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 11:01 AM
 */

#ifndef UNIFORMSEARCH_H
#define	UNIFORMSEARCH_H

#include "trypatch.h"
#include "../im/patch.h"
#include "../sampling/uniform.h"

namespace pm {
    
    /**
     * Uniform search for NNF improvement
     */
    template < typename NNF, typename Patch = typename NNF::TargetPatch >
    class UniformSearch : public TryPatch<NNF, Patch> {
    public:

        bool operator()(const Point2i &i, bool);

        UniformSearch(NNF *nnf) : TryPatch(nnf){}
    };
    
    // Implementation for 2d translation patches
    template <typename NNF, typename Scalar = int>
    bool UniformSearch< NNF, BasicPatch<Scalar> >::operator()(const Point2i &i, bool){
        typedef typename BasicPatch<Scalar>::point point;
        typedef typename point::vec vec;
        // uniformly sample a position for the new patch
        point q = uniform(
            nnf->rand,
            vec(0, 0),
            vec(nnf->target.width - BasicPatch<Scalar>::width(), nnf->target.height - BasicPatch<Scalar>::width())
        );
        return Base::tryPatch(i, BasicPatch<Scalar>(q));
    }
    
}

#endif	/* UNIFORMSEARCH_H */

