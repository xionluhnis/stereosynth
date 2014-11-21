/* 
 * File:   heap.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 4:20 PM
 */

#ifndef HEAP_H
#define	HEAP_H

#include <functional>

namespace pm {
    
    /**
     * In-place heap for arrays, defaulting to max-heaps (Compare = std::less)
     */
    namespace heap {
        template < typename T, typename Compare = std::less >
        void heapify(T *arr, int N) {
            // @see http://en.wikipedia.org/wiki/Heapsort
            // get index of the parent of last element in the heap
            int start = std::floor((N - 2) / 2);

            while(start >= 0) {
                // sift down the node at index 'start'
                sift_down<T, Compare>(arr, start, N - 1);
                // next parent node
                --start;
            }
        }

        template < typename T, typename Compare = std::less >
        void sift_down(T *arr, int start, int end) {
            int root = start;
            // as long as the root has one child
            while(root * 2 + 1 <= end) {
                int child = root * 2 + 1;
                // find target to swap with
                int swapTarget = root;
                // is it the first child?
                if(Compare(arr[swapTarget], arr[child])){
                    swapTarget = child;
                }
                // is it the right child if it exists?
                if(child + 1 <= end && Compare(arr[swapTarget], arr[child + 1])){
                    swapTarget = child + 1;
                }
                // swapping if there is a need
                if(swapTarget != root){
                    std::swap(arr[root], arr[swapTarget]);
                    // the swapped location might have children to swap with
                    root = swapTarget;
                }
            }
        }
        
        template < typename T, typename Compare = std::less >
        bool heap_insert(T *arr, int N, const T& elem) {
            // @see http://stackoverflow.com/questions/2933758/priority-queue-with-limited-space-looking-for-a-good-algorithm
            // check if the heap top is larger
            if(Compare(elem, arr[0])){
                arr[0] = elem; // replace the top
                sift_down<T, Compare>(arr, 0, N - 1); // sift down the new element
            }
            return false; // no, the element is larger
        }
    }
}

#endif	/* HEAP_H */

