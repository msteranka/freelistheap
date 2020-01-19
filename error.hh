#ifndef __ERROR_H
#define __ERROR_H

#include <cstdio>
#include <cstdlib>

inline void fatal(char *msg) { /* Print error and terminate process */
	if (msg == NULL) { // If msg is NULL, then print system error message
		perror("ERROR");
	} else {
		printf("%s\n", msg); // Otherwise, print msg
	}
	exit(EXIT_FAILURE);
}

#endif
