#ifndef SDL_BITS_TEST_H
#define SDL_BITS_TEST_H

#include <stdio.h>

static inline void
print_failure(char *file, int line)
{
	fprintf(stderr, "FAILURE: %s:%d", file, line);
}

#endif /* SDL_BITS_TEST_H */
