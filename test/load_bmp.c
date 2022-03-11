#include <assert.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include "bmp.h"

static char *const BMP_FILE = "./assets/sample_24bit.bmp";
static char *const MODE     = "r";

int
main(int argc, char *argv[])
{
	int                       ret                  = 1;
	FILE                     *file_h               = NULL;
	bmp_file_header_t        *file_header_h        = NULL;
	bmp_bitmap_info_header_t *bitmap_info_header_h = NULL;
	size_t                    dib_header_size;
	size_t                    reads;

	(void)argc;
	(void)argv;

	file_h = fopen(BMP_FILE, MODE);
	if (file_h == NULL) {
		goto cleanup;
	}

	file_header_h = malloc(sizeof(bmp_file_header_t));
	if (file_header_h == NULL) {
		goto cleanup;
	}

	reads = fread(file_header_h, sizeof(bmp_file_header_t), 1, file_h);
	if (reads != 1) {
		goto cleanup;
	}

	dib_header_size = file_header_h->dib_header_size;
	if (dib_header_size != (size_t)BITMAPINFOHEADER) {
		fprintf(stderr, "Unhandled DIB Header Size: %zu", dib_header_size);
		goto cleanup;
	}

	bitmap_info_header_h = malloc(sizeof(bmp_bitmap_info_header_t));
	if (bitmap_info_header_h == NULL) {
		goto cleanup;
	}

	reads = fread(bitmap_info_header_h, sizeof(bmp_bitmap_info_header_t), 1, file_h);
	if (reads != 1) {
		goto cleanup;
	}

	char *image_data = malloc(sizeof(char) * bitmap_info_header_h->image_size_bytes);
	reads = fread(image_data, sizeof(char) * bitmap_info_header_h->image_size_bytes, 1, file_h);
	if (reads != 1) {
		goto cleanup;
	}

	bmp_pixel_RGB24_t *pixel = (bmp_pixel_RGB24_t *)image_data + 1;
	assert(pixel->blue == 255);
	assert(pixel->green == 255);
	assert(pixel->red == 255);
	(void)pixel;

	ret = 0;

cleanup:
	free(file_header_h);
	fclose(file_h);

	return ret;
}
