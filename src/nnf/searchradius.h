/* 
 * File:   searchradius.h
 * Author: akaspar
 *
 * Created on December 2, 2014, 9:49 AM
 */

#ifndef SEARCHRADIUS_H
#define	SEARCHRADIUS_H

#include <vector>

namespace pm {
    
    template < typename S >
    struct SearchRadius {
        
        SearchRadius() : radius(100), minimum(5), decreaseFactor(2){}
        
        S radius;
        S minimum;
        double decreaseFactor;
        
        static SearchRadius<S> *create(){
            radiuses.push_back(SearchRadius<S>());
            return &radiuses.back();
        }
        
    private:
        static std::vector< SearchRadius<S> > radiuses;
    };
    
#define SEARCH_RADIUS_REGISTER(type) \
    std::vector< SearchRadius<type> > SearchRadius<type>::radiuses;
    
    template < typename S >
    struct DecreasingSearchRadius {
        
        DecreasingSearchRadius() : search(0) {}
        explicit DecreasingSearchRadius(SearchRadius<S> *sr) : search(sr) {}
            
        bool operator()(int, bool){
            search->radius = std::max(search->minimum, S(std::ceil(search->radius / search->decreaseFactor)));
            return false;
        }
        
    private:
        SearchRadius<S> *search;
    };
    
}

#endif	/* SEARCHRADIUS_H */

