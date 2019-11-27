#ifndef __ERROR_H
#define __ERROR_H

#include <stdio.h>
#include <stdlib.h>

inline void fatal(char *msg) {
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}

inline void panic() {
	perror("ERROR");
	exit(EXIT_FAILURE);
}

#endif
