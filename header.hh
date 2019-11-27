#ifndef __HEADER_H
#define __HEADER_H

#define SMALL 0xDEADBEEF
#define LARGE 0xFAACFAAC

typedef struct {
	unsigned char i;
	unsigned int magic;   
} header_t;

typedef struct {
	unsigned int size, magic;
} large_header_t;

#endif
