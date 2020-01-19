#ifndef __HEAP_H
#define __HEAP_H

#include <unistd.h>
#include "freelist.hh"
#include "header.hh"
#include "error.hh"

/*
 * Used for counting number of trailing zero bits
 * http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightModLookup
*/
const unsigned char mod37BitPosition[] = {
	32, 0, 1, 26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11, 0, 13, 4,
	7, 17, 0, 25, 22, 31, 15, 29, 10, 12, 6, 0, 21, 14, 9, 5,
	20, 8, 19, 18
};

class Heap {
	public:
		inline Heap();
		inline void *malloc(size_t);
		inline void free(void *);
	
	private:
		FreeList lists[12];
		const unsigned char NUM_LISTS = 12; // 16B to 32kB size classes
		const size_t THRESHOLD = 8 << NUM_LISTS; // Size threshold at which mmap is called directly
		long pageSize;
		pthread_mutex_t expandLock;

		inline void *mallocLargeObj(size_t);
		inline unsigned char getNearestPow(size_t);
};

inline Heap::Heap() {
	pageSize = sysconf(_SC_PAGESIZE); // Fetch system page size
	if (pageSize == -1) { 
		fatal(NULL);
	}
	expandLock = PTHREAD_MUTEX_INITIALIZER;
	void *addr;
	size_t sizeClass = 16; // Current size class being initialized
	const size_t INIT_SIZE = pageSize << 5; // Initialize each free list with 32 pages

	for (int i = 0; i < NUM_LISTS; ++i) {
		addr = mmap(NULL, INIT_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED) {
			fatal(NULL);
		}
		lists[i] = FreeList(addr, INIT_SIZE, sizeClass, 32, i); // Initialize free list
		sizeClass <<= 1;
	}
}

/*
 * Returns power of nearest power of two of n 
 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
*/
inline unsigned char Heap::getNearestPow(size_t n) {
	n--; // Round to highest power of two
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return mod37BitPosition[(-n & n) % 37]; // Count trailing zero bits
}

// Directly calls mmap
inline void *Heap::mallocLargeObj(size_t size) {
	large_header_t *obj = (large_header_t *) mmap(NULL, size + sizeof(large_header_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (obj == MAP_FAILED) return NULL;
	obj->size = size;
	obj->magic = LARGE; // Mark object as being "large"
	return obj + 1;
}

inline void *Heap::malloc(size_t size) {
	if (size > THRESHOLD) { // If requested size exceeds object size threshold, then call mmap directly
		return mallocLargeObj(size);
	}

	unsigned char i = getNearestPow(size) - 4; // Fetch nearest power of two, subtract 4 since first free list contains objects up to 16 bytes
	void *addr = lists[i].malloc();

	if (addr == NULL) { // If free list is empty, then expand it
		unsigned int numPages = lists[i].getNumPages();
		pthread_mutex_lock(&expandLock);
		if (numPages == lists[i].getNumPages()) { // If list hasn't been expanded
			void *newAddr = mmap(NULL, numPages * pageSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
			if (newAddr == MAP_FAILED) return NULL;
			lists[i].expand(newAddr, numPages * pageSize); // Double number of pages
			lists[i].setNumPages(numPages << 1);
		}
		pthread_mutex_unlock(&expandLock);
		addr = lists[i].malloc();
	}

	return addr;
}

inline void Heap::free(void *ptr) {
	header_t *obj = (header_t *) ptr - 1; // Fetch metadata

	if (obj->magic == LARGE) { // If object is "large", then cast and call munmap directly
		large_header_t *largeObj = (large_header_t *) obj;
		if (munmap(largeObj, largeObj->size + sizeof(large_header_t)) == -1) {
			fatal(NULL);
		}
	} else if (obj->magic == SMALL) { // If "small", then call free for respective free list
		lists[obj->i].free(ptr);
	} else {
		fatal((char *) "ERROR: INVALID OBJECT IN FREE()"); // Cast to suppress warning
	}
}

#endif
