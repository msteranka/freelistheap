#ifndef __BAG_H
#define __BAG_H

#include <stdlib.h>
#include "header.h"

class Bag {
	public:
		Bag();
		Bag(void *, size_t);
		void *alloc(int);
		void dealloc(void *);
		void expand(void *);
		size_t getSize();

	private:
		typedef struct __node_t {
			size_t sz;            
			struct __node_t *next;  
		} node_t;

		size_t sz;
		node_t *head;
};

Bag::Bag() {}

Bag::Bag(void *ptr, size_t sz) {
	// std::cout << sizeof(header_t) << ", " << sizeof(node_t) << std::endl;
	this->sz = sz;
	head = NULL;
	expand(ptr);
}

void Bag::expand(void *ptr) {
	node_t *cur;

	for (int i = 0; i < 4096; i += sz + sizeof(node_t)) {
		cur = (node_t *) ((char *) ptr + i);
		cur->sz = sz;
		cur->next = head;
		head = cur;
	}
}

void *Bag::alloc(int i) {
	if (head == NULL) {
		return NULL;
	}

	header_t *tmp = (header_t *) head;
	tmp->i = i;
	tmp->magic = MAGIC;
	head = head->next;
	return (void *) (tmp + 1);
}

void Bag::dealloc(void *ptr) {
	node_t *node = (node_t *) ptr;
	node->sz = sz;
	node->next = head;
	head = node;
}

size_t Bag::getSize() {
	return sz;
}

#endif
