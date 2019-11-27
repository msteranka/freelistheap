#ifndef __FREE_LIST_H
#define __FREE_LIST_H

#include <pthread.h>
#include "header.hh"

class FreeList {
	public:
		inline FreeList();
		inline FreeList(void *, size_t, unsigned int, unsigned int, unsigned char);
		inline void *malloc();
		inline void free(void *);
		inline void expand(void *, const size_t);
		inline unsigned int getSize();
		inline unsigned int getNumPages();
		inline void setNumPages(unsigned int);

	private:
		typedef struct __node_t {
			struct __node_t *next;  
		} node_t;

		node_t *head;
		unsigned int sizeClass, numPages;
		unsigned char index;
		pthread_mutex_t headLock, numPagesLock;
};

inline FreeList::FreeList() {}

inline FreeList::FreeList(void *addr, size_t length, unsigned int sizeClass, unsigned int numPages, unsigned char index) {
	head = NULL;
	this->sizeClass = sizeClass;
	this->numPages = numPages;
	this->index = index;
	headLock = numPagesLock = PTHREAD_MUTEX_INITIALIZER;
	expand(addr, length);
}

inline void FreeList::expand(void *ptr, const size_t LEN) {
	node_t *cur;
	const size_t TOTAL_SIZE = sizeClass + sizeof(node_t);
	const size_t AVAILABLE = LEN - TOTAL_SIZE;

	for (int i = 0; i < AVAILABLE; i += TOTAL_SIZE) {
		cur = (node_t *) ((char *) ptr + i);
		pthread_mutex_lock(&headLock);
		cur->next = head;
		head = cur;
		pthread_mutex_unlock(&headLock);
	}
}

inline void *FreeList::malloc() {
	pthread_mutex_lock(&headLock);
	if (head == NULL) {
		pthread_mutex_unlock(&headLock);
		return NULL;
	}
	header_t *obj = (header_t *) head;
	head = head->next;
	pthread_mutex_unlock(&headLock);
	obj->i = index;
	obj->magic = SMALL;
	return (void *) (obj + 1);
}

inline void FreeList::free(void *ptr) {
	node_t *node = (node_t *) ptr;
	pthread_mutex_lock(&headLock);
	node->next = head;
	head = node;
	pthread_mutex_unlock(&headLock);
}

inline unsigned int FreeList::getSize() {
	return sizeClass;
}

inline unsigned int FreeList::getNumPages() {
	pthread_mutex_lock(&numPagesLock);
	unsigned int val = numPages;
	pthread_mutex_unlock(&numPagesLock);
	return val;
}

inline void FreeList::setNumPages(unsigned int numPages) {
	pthread_mutex_lock(&numPagesLock);
	this->numPages = numPages;
	pthread_mutex_unlock(&numPagesLock);
}

#endif
