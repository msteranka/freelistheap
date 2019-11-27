#ifndef __HEAP_H
#define __HEAP_H

#include <unistd.h>
#include "freelist.h"
#include "header.h"
#include "error.h"

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
		const size_t THRESHOLD = 8 << NUM_LISTS;
		long pageSize;
		pthread_mutex_t expandLock;

		inline void *mallocLargeObj(size_t);
		inline unsigned char getNearestPow(size_t);
};

inline Heap::Heap() {
	pageSize = sysconf(_SC_PAGESIZE);
	if (pageSize == -1) { 
		panic();
	}
	expandLock = PTHREAD_MUTEX_INITIALIZER;
	void *addr;
	size_t sizeClass = 16;
	const size_t INIT_SIZE = pageSize << 5; // Initialize each bag with 32 pages

	for (int i = 0; i < NUM_LISTS; ++i) {
		addr = mmap(NULL, INIT_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED) {
			panic();
		}
		lists[i] = FreeList(addr, INIT_SIZE, sizeClass, 32, i);
		sizeClass <<= 1;
	}
}

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

inline void *Heap::mallocLargeObj(size_t size) {
	large_header_t *obj = (large_header_t *) mmap(NULL, size + sizeof(large_header_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (obj == MAP_FAILED) return NULL;
	obj->size = size;
	obj->magic = LARGE;
	return obj + 1;
}

inline void *Heap::malloc(size_t size) {
	if (size > THRESHOLD) {
		return mallocLargeObj(size);
	}

	unsigned char i = getNearestPow(size) - 4;
	void *addr = lists[i].malloc();

	if (addr == NULL) { 
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
	header_t *obj = (header_t *) ptr - 1;

	if (obj->magic == LARGE) {
		large_header_t *largeObj = (large_header_t *) obj;
		if (munmap(largeObj, largeObj->size + sizeof(large_header_t)) == -1) {
			panic();
		}
	} else if (obj->magic == SMALL) {
		lists[obj->i].free(ptr);
	} else {
		fatal((char *) "ERROR: INVALID OBJECT IN FREE()"); // Cast to suppress warning
	}
}

#endif
