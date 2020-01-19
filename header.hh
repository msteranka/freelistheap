#ifndef __HEADER_H
#define __HEADER_H

#define SMALL 0xDEADBEEF // Magic number for small objects
#define LARGE 0xFAACFAAC // Magic number for large objects

typedef struct {
	unsigned char i; // Index for corresponding free list
	unsigned int magic;
} header_t;

typedef struct {
	unsigned int size, magic;
} large_header_t;

#endif
