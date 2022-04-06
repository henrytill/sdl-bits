#include <stdint.h>

#include "util.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

uint32_t util_uint32_sat_sub(uint32_t x, uint32_t y) {
    uint32_t ret = x - y;
    ret &= -(ret <= x);
    return ret;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
