#include "heap.hh"
#include "header.hh"
#include "error.hh"
#include "freelist.hh"
#include <unistd.h>

extern "C" {
	void *malloc(size_t);
	void free(void *);
	void *calloc(size_t, size_t);
	void *realloc(void *, size_t);
}


