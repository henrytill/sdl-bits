#ifndef SDL_BITS_TEST_H
#define SDL_BITS_TEST_H

#include <stdio.h>
#include <stdlib.h>

#define test(expr)                                                                                 \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            fprintf(stderr, "FAIL: %s:%d", __FILE__, __LINE__);                                    \
            exit(EXIT_FAILURE);                                                                    \
        }                                                                                          \
    } while (0)

#endif /* SDL_BITS_TEST_H */
