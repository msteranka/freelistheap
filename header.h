#ifndef __HEADER_H
#define __HEADER_H

#define MAGIC 0xDEADBEEF

typedef struct {
	int i;
	unsigned int magic;   
	size_t padding;
} header_t;

#endif
