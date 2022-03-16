#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "bmp.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

static char *const  MODE            = "wb";
static const double BITS_PER_DWORD  = 32;
static const size_t BYTES_PER_DWORD = 4;

size_t
bmp_row_size(uint16_t bits_per_pixel, int32_t width_px)
{
	double bits  = (double)bits_per_pixel;
	double width = (double)width_px;
	size_t ret   = (size_t)(ceil((bits * width) / BITS_PER_DWORD)) * BYTES_PER_DWORD;
	return ret;
}

int
bmp_write_bitmap_v4(bmp_pixel_ARGB32_t *target_buff, size_t image_width, size_t image_height, char *file)
{
	int                      ret                = 1;
	FILE                    *file_h             = NULL;
	bmp_color_space_triple_t color_space_triple = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	bmp_bitmap_v4_header_t   bitmap_v4_header;
	bmp_file_header_t        file_header;
	size_t                   image_size_bytes;
	size_t                   file_size_bytes;
	size_t                   writes;

	assert(bmp_bitmap_v4_offset < UINT32_MAX);

	if (target_buff == NULL) {
		return ret;
	}

	if (file == NULL) {
		return ret;
	}

	if (image_width > INT32_MAX || image_height > INT32_MAX) {
		return ret;
	}

	image_size_bytes = (image_width * image_height) * sizeof(bmp_pixel_ARGB32_t);

	if (image_size_bytes > UINT32_MAX) {
		return ret;
	}

	file_size_bytes = bmp_bitmap_v4_offset + image_size_bytes;

	if (file_size_bytes > UINT32_MAX) {
		return ret;
	}

	bitmap_v4_header.width_px           = (int32_t)image_width;
	bitmap_v4_header.height_px          = (int32_t)image_height;
	bitmap_v4_header.num_planes         = 1;
	bitmap_v4_header.bits_per_pixel     = 32;
	bitmap_v4_header.compression        = 0x0003;
	bitmap_v4_header.image_size_bytes   = (uint32_t)image_size_bytes;
	bitmap_v4_header.x_resolution_ppm   = 0;
	bitmap_v4_header.y_resolution_ppm   = 0;
	bitmap_v4_header.num_colors         = 0;
	bitmap_v4_header.important_colors   = 0;
	bitmap_v4_header.red_mask           = 0x00FF0000;
	bitmap_v4_header.green_mask         = 0x0000FF00;
	bitmap_v4_header.blue_mask          = 0x000000FF;
	bitmap_v4_header.alpha_mask         = 0xFF000000;
	bitmap_v4_header.color_space_type   = 0x57696E20;
	bitmap_v4_header.color_space_triple = color_space_triple;
	bitmap_v4_header.red_gamma          = 0;
	bitmap_v4_header.green_gamma        = 0;
	bitmap_v4_header.blue_gamma         = 0;

	file_header.type            = 0x4D42;
	file_header.size            = (uint32_t)file_size_bytes;
	file_header.reserved1       = 0;
	file_header.reserved2       = 0;
	file_header.offset          = (uint32_t)bmp_bitmap_v4_offset;
	file_header.dib_header_size = BITMAPV4HEADER;

	file_h = fopen(file, MODE);
	if (file_h == NULL) {
		return ret;
	}

	writes = fwrite(&file_header, sizeof(bmp_file_header_t), 1, file_h);
	if (writes != 1) {
		goto cleanup;
	}

	writes = fwrite(&bitmap_v4_header, sizeof(bmp_bitmap_v4_header_t), 1, file_h);
	if (writes != 1) {
		goto cleanup;
	}

	writes = fwrite(target_buff, image_size_bytes, 1, file_h);
	if (writes != 1) {
		goto cleanup;
	}

	ret = 0;

cleanup:
	fclose(file_h);
	return ret;
}
