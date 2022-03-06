#ifndef SDL_BITS_UTIL_H
#define SDL_BITS_UTIL_H

#include <stdint.h>
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

/*
 * Performs saturating subtraction
 *
 * Ref: http://locklessinc.com/articles/sat_arithmetic/
 */
uint32_t
util_uint32_sat_sub(uint32_t x, uint32_t y);

/* A growable buffer */
typedef struct util_buffer_s util_buffer_t;

/* Initialize a growable buffer */
int
util_buffer_init(util_buffer_t **buff, uint32_t cap);

/* De-initialize a growable buffer */
int
util_buffer_deinit(util_buffer_t **buff);

/* Get the capacity of a growable buffer */
uint32_t
util_buffer_cap(util_buffer_t *buff);

/* Get the count of a growable buffer */
uint32_t
util_buffer_count(util_buffer_t *buff);

/* Push an item to a growable buffer */
int
util_buffer_push(util_buffer_t *buff, char item);

/* Set an item in a growable buffer */
int
util_buffer_set(util_buffer_t *buff, uint32_t index, char item);

/* Read an item from a growable buffer */
int
util_buffer_read(util_buffer_t *buff, uint32_t index, char *out);

/* A growable 2d buffer */
typedef struct util_buffer2d_s util_buffer2d_t;

/* Initialize a growable 2d buffer */
int
util_buffer2d_init(util_buffer2d_t **buff, uint32_t x_cap, uint32_t y_cap);

/* De-initialize a growable 2d buffer */
int
util_buffer2d_deinit(util_buffer2d_t **buff);

/* Set an item in a growable 2d buffer */
int
util_buffer2d_set(util_buffer2d_t *buff,
                  uint32_t         x_index,
                  uint32_t         y_index,
                  char             item);

/* Read an item from a growable 2d buffer */
int
util_buffer2d_read(util_buffer2d_t *buff,
                   uint32_t         x_index,
                   uint32_t         y_index,
                   char            *out);

/* Get the column capacity of a growable buffer */
uint32_t
util_buffer2d_x_cap(util_buffer2d_t *buff);

/* Get the row capacity of a growable buffer */
uint32_t
util_buffer2d_y_cap(util_buffer2d_t *buff);

#endif /* SDL_BITS_UTIL_H */
