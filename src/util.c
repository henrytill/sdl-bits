#include "util.h"

uint32_t
util_uint32_sat_sub(uint32_t x, uint32_t y)
{
	uint32_t ret;

	ret = x - y;
	ret &= -(ret <= x);
	return ret;
}
