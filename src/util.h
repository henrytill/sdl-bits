#ifndef SDL_BITS_UTIL_H
#define SDL_BITS_UTIL_H

#include <stdio.h>
#include <string.h>

#include "SDL.h"

/* Prints well-formatted SDL error */

#define util_print_sdl_error()                                                 \
	do {                                                                   \
		const char *sdl_err = SDL_GetError();                          \
		fprintf(                                                       \
		    stderr, "ERROR: %s:%d:%s", __FILE__, __LINE__, __func__);  \
		if (strlen(sdl_err) != 0) {                                    \
			fprintf(stderr, ": %s", sdl_err);                      \
		}                                                              \
		fprintf(stderr, "\n");                                         \
	} while (0)

#endif