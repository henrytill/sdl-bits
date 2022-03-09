#include <math.h>

#include "bmp.h"

const double BITS_PER_DWORD  = 32;
const size_t BYTES_PER_DWORD = 4;

size_t
bmp_row_size(uint16_t bits_per_pixel, int32_t width_px)
{
	double bits  = (double)bits_per_pixel;
	double width = (double)width_px;
	size_t ret   = (size_t)(ceil((bits * width) / BITS_PER_DWORD)) * BYTES_PER_DWORD;
	return ret;
}
