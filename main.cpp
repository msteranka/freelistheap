#include <stdio.h>
#include <iostream>
#include <cassert>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include "freelistheap.h"

void benchmark() {
	FreeListHeap heap;
	const int N = 1000000;
	void *addrs[N];
	int ct = 0;
	for (int i = 0; i < N; ++i) {
		addrs[i] = heap.alloc(16);
	}
	for (int i = 0; i < N; ++i) {
		heap.dealloc(addrs[i]);
	}
}

int main() {
	FreeListHeap heap;
	char *str = (char *) heap.alloc(17);
	printf("%p\n", str);
	strcpy(str, "1");
	// printf("%s, %lu\n", str, strlen(str));
	// heap.dealloc(str);
	return 0;
}
