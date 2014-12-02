/* 
 * File:   algorithm.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 11:18 AM
 */

#ifndef ALGORITHM_H
#define	ALGORITHM_H

#include <functional>

namespace pm {
    
    /**
     * Algorithm sequence wrapper
     */
    struct Algorithm {
        typedef std::function<bool(const Point2i &, bool)> AlgorithmPart;
        bool operator()(const Point2i &i, bool rev) {
            bool res = false;
            for(AlgorithmPart &p : seq){
                res |= p(i, rev);
            }
            return res;
        }

        Algorithm(){}

        Algorithm &operator <<(AlgorithmPart p){
            seq.push_back(p);
            return *this;
        }
    private:
        std::vector<AlgorithmPart> seq;
    };
    
}

#endif	/* ALGORITHM_H */

