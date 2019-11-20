#ifndef __FREE_LIST_H
#define __FREE_LIST_H

#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <sys/mman.h>
#include "bag.h"
#include "header.h"

#define HEAP_SIZE 4096

class FreeListHeap {
	public:
		FreeListHeap();
		void *alloc(size_t);
		void dealloc(void *);
	
	private:
		const int NBAGS = 4;
		Bag bags[4];
};

FreeListHeap::FreeListHeap() {
	void *next;
	size_t sz = 16;
	for (int i = 0; i < NBAGS; ++i) {
		next = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (next == NULL) {
			std::cout << "oh noes" << std::endl;
			exit(-1);
		}
		bags[i] = Bag(next, sz);
		sz *= 2;
	}
}

void *FreeListHeap::alloc(size_t sz) {
	sz--;
	sz |= sz >> 1;
	sz |= sz >> 2;
	sz |= sz >> 4;
	sz |= sz >> 8;
	sz |= sz >> 16;
	sz++;
	int i = 0;
	while (sz >= 2) {
		sz = sz >> 1;	
		++i;
	}
	i -= 4;
	std::cout << i << std::endl;
	void *ptr = bags[i].alloc(i);
	if (ptr == NULL) {
		void *tmp = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (tmp == NULL) {
			std::cout << "oh noes" << std::endl;
			exit(-1);
		}
		bags[i].expand(tmp);
		ptr = bags[i].alloc(i);
	}
	return ptr;
}

void FreeListHeap::dealloc(void *ptr) {
	std::cout << "yay" << std::endl;
	header_t *meta = (header_t *) ptr - 1;
	assert(meta->magic == MAGIC);
	int i = meta->i;
	bags[i].dealloc(ptr);
}

#endif
