#ifndef SDL_BITS_TEST_H
#define SDL_BITS_TEST_H

#include <stdio.h>

#define TEST(expr)                                                                                 \
	do {                                                                                       \
		if (!(expr)) {                                                                     \
			fprintf(stderr, "FAIL: %s:%d", __FILE__, __LINE__);                        \
			return 1;                                                                  \
		}                                                                                  \
	} while (0)

#endif /* SDL_BITS_TEST_H */
