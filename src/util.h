#ifndef SDL_BITS_UTIL_H
#define SDL_BITS_UTIL_H

#include <stdint.h>

/* Ref: http://locklessinc.com/articles/sat_arithmetic/ */
uint32_t util_uint32_sat_sub(uint32_t x, uint32_t y);

#endif /* SDL_BITS_UTIL_H */
