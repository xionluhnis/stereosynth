/* 
 * File:   ringbuffer.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 4:20 PM
 */

#ifndef RINGBUFFER_H
#define	RINGBUFFER_H

#include <vector>

typedef std::size_t size_t;

template <typename T>
class RingBuffer {
public:

	RingBuffer(int size) : emptyFlag(false), first(0), last(0), ptr(0), data() {
		data.resize(size);
	}
	/// return the first element of the buffer

	inline const T &top() const {
		return data[first];
	}
	/// return the first element, removing it

	inline T &pop() {
		int index = first;
		first = (first + 1) % data.size();
		if (first == last) emptyFlag = true;
		if (ptr == index) ptr = first; // shift the pointer
		return data[index];
	}
	/// return the currently pointer element, shifting the pointer to the next available element

	inline T &shift() {
		int index = ptr;
		if (ptr == last) ptr = first;
		else ptr = (first + 1) % data.size();
		return data[index];
	}

	inline void push(const T &elem) {
		if (full()) {
			data[first] = elem;
		} else {
			data[last] = elem;
			last = (last + 1) % data.size();
			emptyFlag = false;
		}
	}

	inline bool empty() const {
		return emptyFlag;
	}

	inline bool full() const {
		return first == 0 ? last == data.size() - 1 : first == last + 1;
	}

	inline size_t size() const {
		if (empty()) return 0;
		if (full()) return data.size();
		return (last - first + 1 + data.size()) % data.size();
	}

	inline size_t capacity() const {
		return data.capacity();
	}

private:
	RingBuffer(); // you need the size!
	bool emptyFlag;
	int first, last, ptr;
	std::vector<T> data;
};

#endif	/* RINGBUFFER_H */

