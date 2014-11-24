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
        template < typename T, typename Compare = std::less<T> >
        void sift_down(T *arr, int start, int end, const Compare &comp = Compare()) {
            int root = start;
            // as long as the root has one child
            while(root * 2 + 1 <= end) {
                int child = root * 2 + 1;
                // find target to swap with
                int swapTarget = root;
                // is it the first child?
                if(comp(arr[swapTarget], arr[child])){
                    swapTarget = child;
                }
                // is it the right child if it exists?
                if(child + 1 <= end && comp(arr[swapTarget], arr[child + 1])){
                    swapTarget = child + 1;
                }
                // swapping if there is a need
                if(swapTarget != root){
                    std::swap(arr[root], arr[swapTarget]);
                    // the swapped location might have children to swap with
                    root = swapTarget;
                } else {
                    return;
                }
            }
        }
        
        template < typename T, typename Compare = std::less<T> >
        void heapify(T *arr, int N, const Compare &comp = Compare()) {
            // @see http://en.wikipedia.org/wiki/Heapsort
            // get index of the parent of last element in the heap
            int start = std::floor((N - 2) / 2);

            while(start >= 0) {
                // sift down the node at index 'start'
                sift_down<T, Compare>(arr, start, N - 1, comp);
                // next parent node
                --start;
            }
        }
        
        template < typename T, typename Compare = std::less<T> >
        bool heap_insert(T *arr, int N, const T& elem, const Compare &comp = Compare()) {
            // @see http://stackoverflow.com/questions/2933758/priority-queue-with-limited-space-looking-for-a-good-algorithm
            // check if the heap top is larger
            if(comp(elem, arr[0])){
                arr[0] = elem; // replace the top
                sift_down<T, Compare>(arr, 0, N - 1, comp); // sift down the new element
                return true;
            }
            return false; // no, the element is larger
        }
    }
	
	template < int K, typename T, typename Compare = std::less<T> >
	struct Heap {
		T *data;
		Heap(T *d) : data(d){}
		
		const T& head() const {
			return data[0];
		}
		const T &tail() const {
			return data[K-1];
		}
		
		bool insert(const T &elem) {
			return heap::heap_insert<T, Compare>(data, K, elem, comp);
		}
    private:
        Compare comp;
	};
}

#endif	/* HEAP_H */

