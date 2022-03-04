#ifndef SDL_BITS_UTIL_H
#define SDL_BITS_UTIL_H

#include <stdio.h>
#include <string.h>

#define BUFFER_GROWTH_FACTOR 2

/* Prints well-formatted SDL error */
#define util_print_sdl_error()                                                 \
	do {                                                                   \
		const char *sdl_err = SDL_GetError();                          \
		fprintf(stderr, "ERROR: %s:%d", __FILE__, __LINE__);           \
		if (strlen(sdl_err) != 0) {                                    \
			fprintf(stderr, ": %s", sdl_err);                      \
		}                                                              \
		fprintf(stderr, "\n");                                         \
	} while (0)

/* Define uint32_t for compatibility with SDL typedef */
typedef unsigned int uint32_t;

/*
 * Performs saturating subtraction
 *
 * Ref: http://locklessinc.com/articles/sat_arithmetic/
 */
uint32_t
util_uint32_sat_sub(uint32_t x, uint32_t y);

/* A growable buffer */
typedef struct util_buffer_s {
	size_t cap;
	size_t size;
	char  *data;
} util_buffer_t;

/* Initialize a growable buff */
int
util_buffer_init(util_buffer_t *buff, size_t cap);

/* De-initialize a growable buff */
int
util_buffer_deinit(util_buffer_t *buff);

/* Push an item to a growable buffer */
int
util_buffer_push(util_buffer_t *buff, char item);

/* Set an item in a growable buffer */
int
util_buffer_set(util_buffer_t *buff, size_t index, char item);

/* Read an item from a growable buff */
int
util_buffer_read(util_buffer_t *buff, size_t index, char *out);

#endif /* SDL_BITS_UTIL_H */