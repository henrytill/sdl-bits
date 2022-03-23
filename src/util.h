#ifndef SDL_BITS_UTIL_H
#define SDL_BITS_UTIL_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { BUFFER_GROWTH_FACTOR = 2 };

// Prints well-formatted SDL error
#define util_print_sdl_error()                                                                     \
    do {                                                                                           \
        const char *sdl_err = SDL_GetError();                                                      \
        fprintf(stderr, "ERROR: %s:%d", __FILE__, __LINE__);                                       \
        if (strlen(sdl_err) != 0) {                                                                \
            fprintf(stderr, ": %s", sdl_err);                                                      \
        }                                                                                          \
        fprintf(stderr, "\n");                                                                     \
    } while (0)

//
//  Performs saturating subtraction
//
//  Ref: http://locklessinc.com/articles/sat_arithmetic/
//
uint32_t util_uint32_sat_sub(uint32_t x, uint32_t y);

// A growable buffer
typedef struct util_Buffer util_Buffer;

// Initialize a growable buffer
int util_buffer_init(util_Buffer **buff, uint32_t cap);

// De-initialize a growable buffer
int util_buffer_deinit(util_Buffer **buff);

// Get the capacity of a growable buffer
uint32_t util_buffer_cap(util_Buffer *buff);

// Get the count of a growable buffer
uint32_t util_buffer_count(util_Buffer *buff);

// Push an item to a growable buffer
int util_buffer_push(util_Buffer *buff, char item);

// Set an item in a growable buffer
int util_buffer_set(util_Buffer *buff, uint32_t index, char item);

// Read an item from a growable buffer
int util_buffer_read(util_Buffer *buff, uint32_t index, char *out);

// A growable 2d buffer
typedef struct util_Buffer2d util_Buffer2d;

// Initialize a growable 2d buffer
int util_buffer2d_init(util_Buffer2d **buff, uint32_t x_cap, uint32_t y_cap);

// De-initialize a growable 2d buffer
int util_buffer2d_deinit(util_Buffer2d **buff);

// Get the column capacity of a growable buffer
uint32_t util_buffer2d_x_cap(util_Buffer2d *buff);

// Get the row capacity of a growable buffer
uint32_t util_buffer2d_y_cap(util_Buffer2d *buff);

// Set an item in a growable 2d buffer
int util_buffer2d_set(util_Buffer2d *buff, uint32_t x_index, uint32_t y_index, char item);

// Read an item from a growable 2d buffer
int util_buffer2d_read(util_Buffer2d *buff, uint32_t x_index, uint32_t y_index, char *out);

#endif // SDL_BITS_UTIL_H
