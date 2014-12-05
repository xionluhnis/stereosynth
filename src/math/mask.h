/* 
 * File:   mask.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 18, 2014, 7:20 AM
 */

#ifndef IM_MASK_H
#define	IM_MASK_H

#include "mat.h"

namespace pm {
    
    typedef Mat Mask;
    
    template< typename T, bool Result = false >
    struct MaskFilter {
        
        /**
         * Filter index
         */
        unsigned int operator()(const T &p, bool) const {
            if(!mask.empty()){
                return mask.at<float>(p.y, p.x) != 0.0f;
            }
            return 0.0f;
        }
        
        explicit MaskFilter(const Mat &m) : mask(m) {}
        
    private:
        const Mask mask;
    };
    
}

#endif	/* IM_MASK_H */

